//=============================================================================
// Copyright (C) 2003-2007 ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "CgProgramInfo.h"
#include "Renderer.h"

#include <y60/glutil/GLUtils.h>


#ifdef WIN32
#   include <GL/glh_extensions.h>
#   include <GL/glh_genext.h>
#endif
#ifdef LINUX
    #include <GL/glext.h>
    #include <GL/glx.h>
#endif

#include <y60/base/NodeValueNames.h>
#include <asl/base/string_functions.h>
#include <asl/dom/Nodes.h>

#define DB(x) // x

// profiling
#ifdef PROFILING_LEVEL_NORMAL
#warning PROFILING_LEVEL_NORMAL is enabled
#define DBP(x)  x
#else
#define DBP(x) // x
#endif

// more profiling
//#define PROFILING_LEVEL_FULL
#ifdef PROFILING_LEVEL_FULL
#warning PROFILING_LEVEL_FULL is enabled
#define DBP2(x)  x
#else
#define DBP2(x) // x
#endif

using namespace std;
using namespace asl;
using namespace dom;

namespace y60 {

    void
    assertCg(const std::string & theWhere, const CGcontext theCgContext) {
        CGerror myCgError = cgGetError();
        if (myCgError == CG_COMPILER_ERROR && theCgContext) {
            throw RendererException(std::string("Cg compile error: ") + cgGetLastListing(theCgContext),
                                    theWhere);
        } else if (myCgError != CG_NO_ERROR) {
            throw RendererException(cgGetErrorString(myCgError), theWhere);
        }
    }

#ifdef DEBUG
#define ASSERTCG(where, what) assertCg(where, what)
#else
#define ASSERTCG(where, what) 
#endif

    CgProgramInfo::CgProgramInfo(const ShaderDescription & myShader,
                                 const CGcontext theCgContext)
        : _myShader(myShader), _myContext(theCgContext)
    {
        _myPathName = myShader._myFilename;

        _myUnsizedArrayAutoParamSizes[POSITIONAL_LIGHTS] = 0;
        _myUnsizedArrayAutoParamSizes[POSITIONAL_LIGHTS_DIFFUSE_COLOR] = 0;
        _myUnsizedArrayAutoParamSizes[POSITIONAL_LIGHTS_SPECULAR_COLOR] = 0;

        _myUnsizedArrayAutoParamSizes[DIRECTIONAL_LIGHTS] = 0;
        _myUnsizedArrayAutoParamSizes[DIRECTIONAL_LIGHTS_DIFFUSE_COLOR] = 0;
        _myUnsizedArrayAutoParamSizes[DIRECTIONAL_LIGHTS_SPECULAR_COLOR] = 0;

        _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS] = 0;
        _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_DIFFUSE_COLOR] = 0;
        _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_SPECULAR_COLOR] = 0;
        _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_CUTOFF] = 0;
        _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_EXPONENT] = 0;
        _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_DIRECTION] = 0;
        _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_ATTENUATION] = 0;
        
        _myUnsizedArrayAutoParamSizes[TEXTURE_MATRICES] = 0;

        //VS: 100 punkte
        cgSetAutoCompile(_myContext, CG_COMPILE_MANUAL);
        createAndCompileProgram();

        DB(AC_TRACE << "CgProgramInfo::CgProgramInfo(" << _myCgProgramID << ")" << endl;);
    }

    CgProgramInfo::~CgProgramInfo() {
        if (cgIsProgram(_myCgProgramID)) {
            if (cgGetProgramContext(_myCgProgramID) == _myContext) {
                DB(AC_TRACE << "CgProgramInfo::~CgProgramInfo(" << _myCgProgramID << ")" << endl;);
                cgDestroyProgram(_myCgProgramID);
            }
            assertCg("CgProgramInfo::~CgProgramInfo()", _myContext);
        }
    }

    void CgProgramInfo::createAndCompileProgram() {
        AC_DEBUG << "createAndCompileProgram " << _myPathName.c_str();
        DBP2(MAKE_GL_SCOPE_TIMER(CgProgramInfo_createAndCompileProgram));

        if (_myCgProgramString.empty()) {
            AC_DEBUG << "loading from file";
            DBP2(MAKE_GL_SCOPE_TIMER(CgProgramInfo_loadFromFile));

            // create null terminated array of null terminated strings
            for (int i=0; i < _myShader._myCompilerArgs.size(); ++i) {
                AC_DEBUG << "Using arg " << _myShader._myCompilerArgs[i].c_str() << endl;
                _myCachedCompilerArgs.push_back(_myShader._myCompilerArgs[i].c_str());
            }
            _myCachedCompilerArgs.push_back(0);

            DBP2(START_TIMER(CgProgramInfo_cgCreateProgramFromFile));
            _myCgProgramID = cgCreateProgramFromFile(_myContext, CG_SOURCE,
                                                     _myPathName.c_str(),
                                                     asCgProfile(_myShader),
                                                     _myShader._myEntryFunction.c_str(),
                                                     &(*_myCachedCompilerArgs.begin()));
            DBP2(STOP_TIMER(CgProgramInfo_cgCreateProgramFromFile));

            assertCg(PLUS_FILE_LINE, _myContext);

            //DK: we cannot keep a compiled version
            //    because unsized arrays of size 0 are completely gone after compilation
            _myCgProgramString = cgGetProgramString(_myCgProgramID, CG_PROGRAM_SOURCE);

        } else {
            AC_DEBUG << "destroying and reloading from string";
            DBP2(MAKE_GL_SCOPE_TIMER(CgProgramInfo_destroyReload));

            cgDestroyProgram(_myCgProgramID);
            _myCgProgramID = cgCreateProgram(_myContext, CG_SOURCE, _myCgProgramString.c_str(),
                                             asCgProfile(_myShader), _myShader._myEntryFunction.c_str(),
                                             &(*_myCachedCompilerArgs.begin()));
            assertCg(PLUS_FILE_LINE, _myContext);
        }

        DBP2(START_TIMER(CgProgramInfo_processParameters));
        processParameters();
        DBP2(STOP_TIMER(CgProgramInfo_processParameters));

        DBP2(MAKE_GL_SCOPE_TIMER(CgProgramInfo_compileProgram));
        cgCompileProgram(_myCgProgramID);
        assertCg(PLUS_FILE_LINE, _myContext);
    }

    void
    CgProgramInfo::load() {
        cgGLLoadProgram(_myCgProgramID);
        assertCg(PLUS_FILE_LINE, _myContext);
    }

    void
    CgProgramInfo::processParameters() {
        AC_TRACE << "processParameters";

        _myGlParams.clear();
        _myAutoParams.clear();
        _myTextureParams.clear();
        _myMiscParams.clear();
        
        for (CGparameter myParam = cgGetFirstParameter(_myCgProgramID, CG_PROGRAM);
             myParam != 0;  myParam = cgGetNextParameter(myParam))
        {
            string myParamName(cgGetParameterName(myParam));
            //XXX cgGetParameterVariability crashes on unsized arrays
            //CGenum myParamVariability = cgGetParameterVariability(myParam);
            CGenum myParamVariability = CG_UNIFORM;
            CGtype myParameterType    = cgGetParameterType(myParam);
            AC_TRACE << "processing " << myParamName;

            if (myParamName.compare(0, 3, "GL_") == 0) { // scan for reserved GL_ prefix in uniform parameters
                if (myParamVariability != CG_UNIFORM) {
                    throw RendererException ("Error in " + _myPathName + ": Parameter " + myParamName
                                             + " not uniform.", "CgProgramInfo::processParameters()");
                }

                CGGLenum myParamTransform = CG_GL_MATRIX_IDENTITY;
                string myParamBase = myParamName;
                if (myParamName.substr(myParamName.length()-2) == "_I") {
                    myParamTransform = CG_GL_MATRIX_INVERSE;
                    myParamBase = myParamName.substr(0, myParamName.length()-2);
                } else if (myParamName.substr(myParamName.length()-2) == "_T") {
                    myParamTransform = CG_GL_MATRIX_TRANSPOSE;
                    myParamBase = myParamName.substr(0, myParamName.length()-2);
                } else if (myParamName.substr(myParamName.length()-3) == "_IT") {
                    myParamTransform = CG_GL_MATRIX_INVERSE_TRANSPOSE;
                    myParamBase = myParamName.substr(0, myParamName.length()-3);
                }
                CGGLenum myParamType;
                if (myParamBase == CG_GL_MODELVIEW_PARAMETER) {
                    myParamType = CG_GL_MODELVIEW_MATRIX;
                } else if (myParamBase == CG_GL_PROJECTION_PARAMETER) {
                    myParamType = CG_GL_PROJECTION_MATRIX;
                } else if (myParamBase == CG_GL_TEXTURE_PARAMETER) {
                    myParamType = CG_GL_TEXTURE_MATRIX;
                } else if (myParamBase == CG_GL_MODELVIEW_PROJECTION_PARAMETER) {
                    myParamType = CG_GL_MODELVIEW_PROJECTION_MATRIX;
                } else {
                    throw RendererException ("Error in " + _myPathName +
                                             ": CGGL Parameter base " + myParamBase + " unknown.",
                                             "CgProgramInfo::processParameters()");
                }
                _myGlParams.push_back(
                    CgProgramGlParam(myParamName, myParam, myParamType, myParamTransform));
            } else if (myParamName.compare(0, 3, "AC_") == 0) { // scan for reserved AC_ prefix
                if (myParamVariability != CG_UNIFORM) {
                    throw RendererException ("Error in " + _myPathName + ": Parameter " + myParamName
                                             + " not uniform.", "CgProgramInfo::processParameters()");
                }

                CgAutoParameterID myParamID = static_cast<CgAutoParameterID>(
                    asl::getEnumFromString(myParamName, CgAutoParameterString));

                if (myParamID >= MAX_AUTO_PARAMETER) {
                    throw RendererException ("Error in " + _myPathName +
                                             ": CG Auto Parameter " + myParamName + " unknown.",
                                             "CgProgramInfo::processParameters()");
                }

                if (myParameterType == CG_ARRAY) /* && cgGetArraySize(myParam, 0) == 0)*/ {
                    int myArraySize = _myUnsizedArrayAutoParamSizes[myParamID];
                    AC_TRACE << "setting array " << myParamName << " to size " << myArraySize;

                    CGtype myArrayType = cgGetArrayType(myParam);
                    CGparameter myArray
                        = cgCreateParameterArray(_myContext, myArrayType, myArraySize ? myArraySize : 1);
                    assertCg(PLUS_FILE_LINE, _myContext);

                    for(int i = 0; i <  myArraySize; ++i) {

                        CGparameter myArrayElement
                            = cgCreateParameter(_myContext, myArrayType);
                        cgConnectParameter(myArrayElement, cgGetArrayParameter(myArray, i));
                    }

                    cgConnectParameter(myArray, myParam);
                    assertCg(PLUS_FILE_LINE, _myContext);
                    AC_TRACE << "done. created unsized array of size " << myArraySize;
                }
                AC_TRACE << "adding auto param " << myParamName << ":" << myParamID << endl;
                _myAutoParams[myParamID] =
                    CgProgramAutoParam(myParamName, myParam, myParamID, myParameterType);
            } else if (myParameterType == CG_SAMPLER1D ||
                       // scan for sampler params
                       myParameterType == CG_SAMPLER2D ||
                       myParameterType == CG_SAMPLER3D ||
                       myParameterType == CG_SAMPLERCUBE ) {
                _myTextureParams.push_back(CgProgramNamedParam(myParamName, myParam));
            } else {
                _myMiscParams.push_back(CgProgramNamedParam(myParamName, myParam));
            }
            assertCg(PLUS_FILE_LINE, _myContext);
        }
    }

    void
    CgProgramInfo::enableProfile() {
        //AC_TRACE << "enabled CgProfile(" << asCgProfile(_myShader) << ")" << endl;
        cgGLEnableProfile(asCgProfile(_myShader));
        assertCg(PLUS_FILE_LINE, _myContext);
    }

    void
    CgProgramInfo::disableProfile() {
        //AC_TRACE << "disabled CgProfile(" << asCgProfile(_myShader) << ")" << endl;
        cgGLDisableProfile(asCgProfile(_myShader));
        assertCg(PLUS_FILE_LINE, _myContext);
    }

    void CgProgramInfo::setCGGLParameters() {
        //AC_DEBUG << "CgProgramInfo::setCGGLParameters";
        for (unsigned i=0; i<_myGlParams.size(); ++i) {
            CgProgramGlParam myParam = _myGlParams[i];
            AC_TRACE << "setting CgGL parameter " << myParam._myParamName << " param=" << myParam._myParameter << " type=" << myParam._myStateMatrixType << " transform=" << myParam._myTransform;
            cgGLSetStateMatrixParameter(myParam._myParameter,
                                        myParam._myStateMatrixType,
                                        myParam._myTransform);
            assertCg(PLUS_FILE_LINE, _myContext);
        }
    }

    bool
    CgProgramInfo::reloadIfRequired(
        const LightVector & theLightInstances,
        const MaterialBase & theMaterial)
    {
        DBP2(MAKE_GL_SCOPE_TIMER(CgProgramInfo_reloadIfRequired));
        //AC_DEBUG << "reloadIfRequired";

        DBP2(START_TIMER(CgProgramInfo_reloadIfRequired_count_lights));
        //look at the number of lights to see if reload is req.
        unsigned myPositionalLightCount = 0;
        unsigned myDirectionalLightCount = 0;
        unsigned mySpotLightCount = 0;
        DBP2(COUNT_N(CgProgramInfo_reloadIfRequired_lights, theLightInstances.size()));
        for (unsigned i = 0; i < theLightInstances.size(); ++i) {
            LightPtr myLight = theLightInstances[i];
            if ( ! myLight->get<VisibleTag>()) {
                continue;
            }
            LightSourcePtr myLightSource = myLight->getLightSource();
            switch (myLightSource->getType()) {
            case POSITIONAL:
                ++myPositionalLightCount;
                break;
            case DIRECTIONAL:
                ++myDirectionalLightCount;
                break;
            case SPOT:
                ++mySpotLightCount;
                break;
            case AMBIENT :
                break;
            default :
                AC_WARNING << "Unknown light type for " << myLightSource->get<IdTag>();
            }
        }
        DBP2(STOP_TIMER(CgProgramInfo_reloadIfRequired_count_lights));

        bool myReload = false;

        DBP2(START_TIMER(CgProgramInfo_reloadIfRequired_pos_lights));
        if (myReload == false && _myAutoParams.find(POSITIONAL_LIGHTS) != _myAutoParams.end()) {
            unsigned myLastCount = _myUnsizedArrayAutoParamSizes[POSITIONAL_LIGHTS];
            if (myLastCount != myPositionalLightCount) {
                DBP2(COUNT(CgProgramInfo_posLightCount_differ));
                DBP2(COUNT_N(CgProgramInfo_posLightCountLast, myLastCount));
                DBP2(COUNT_N(CgProgramInfo_posLightCountNew, myPositionalLightCount));
                myReload = true;
            }
        }
        DBP2(STOP_TIMER(CgProgramInfo_reloadIfRequired_pos_lights));

        DBP2(START_TIMER(CgProgramInfo_reloadIfRequired_dir_lights));
        if (myReload == false && _myAutoParams.find(DIRECTIONAL_LIGHTS) != _myAutoParams.end()) {
            unsigned myLastCount = _myUnsizedArrayAutoParamSizes[DIRECTIONAL_LIGHTS];
            if (myLastCount != myDirectionalLightCount)  {
                DBP2(COUNT(CgProgramInfo_dirLightCount_differ));
                DBP2(COUNT_N(CgProgramInfo_dirLightCountLast, myLastCount));
                DBP2(COUNT_N(CgProgramInfo_dirLightCountNew, myDirectionalLightCount));
                myReload = true;
            }
        }
        DBP2(STOP_TIMER(CgProgramInfo_reloadIfRequired_dir_lights));
        
        DBP2(START_TIMER(CgProgramInfo_reloadIfRequired_spot_lights));
        if (myReload == false && _myAutoParams.find(SPOT_LIGHTS) != _myAutoParams.end()) {
            unsigned myLastCount = _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS];
            if (myLastCount != mySpotLightCount)  {
                DBP2(COUNT(CgProgramInfo_spotLightCount_differ));
                DBP2(COUNT_N(CgProgramInfo_spotLightCountLast, myLastCount));
                DBP2(COUNT_N(CgProgramInfo_spotLightCountNew, mySpotLightCount));
                myReload = true;
            }
        }
        DBP2(STOP_TIMER(CgProgramInfo_reloadIfRequired_spot_lights));

        if (myReload == false && _myAutoParams.find(TEXTURE_MATRICES) != _myAutoParams.end()) {
            unsigned myLastCount = _myUnsizedArrayAutoParamSizes[TEXTURE_MATRICES];
            if (myLastCount != theMaterial.getTextureUnitCount()) {
                myReload = true;
            }
        }

        if (myReload) {
            //change unsized array sizes here
            AC_DEBUG << "Cg program reload required";

            DBP2(MAKE_GL_SCOPE_TIMER(CgProgramInfo_reloadIfRequired_reload));
            AC_DEBUG << "#of positional lights:" << myPositionalLightCount;
            _myUnsizedArrayAutoParamSizes[POSITIONAL_LIGHTS] = myPositionalLightCount;
            _myUnsizedArrayAutoParamSizes[POSITIONAL_LIGHTS_DIFFUSE_COLOR] = myPositionalLightCount;
            _myUnsizedArrayAutoParamSizes[POSITIONAL_LIGHTS_SPECULAR_COLOR] = myPositionalLightCount;

            AC_DEBUG << "#of directional lights:" << myDirectionalLightCount;
            _myUnsizedArrayAutoParamSizes[DIRECTIONAL_LIGHTS] = myDirectionalLightCount;
            _myUnsizedArrayAutoParamSizes[DIRECTIONAL_LIGHTS_DIFFUSE_COLOR] = myDirectionalLightCount;
            _myUnsizedArrayAutoParamSizes[DIRECTIONAL_LIGHTS_SPECULAR_COLOR] = myDirectionalLightCount;

            AC_DEBUG << "#of spot lights:" << mySpotLightCount;
            _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS] = mySpotLightCount;
            _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_DIFFUSE_COLOR] = mySpotLightCount;
            _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_SPECULAR_COLOR] = mySpotLightCount;
            _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_CUTOFF] = mySpotLightCount;
            _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_EXPONENT] = mySpotLightCount;
            _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_DIRECTION] = mySpotLightCount;
            _myUnsizedArrayAutoParamSizes[SPOT_LIGHTS_ATTENUATION] = mySpotLightCount;

            AC_DEBUG << "#of texture matrices:" << theMaterial.getTextureUnitCount();
            _myUnsizedArrayAutoParamSizes[TEXTURE_MATRICES] = theMaterial.getTextureUnitCount();

            DBP2(START_TIMER(CgProgramInfo_reloadIfRequired_reload_compile));
            createAndCompileProgram();
            DBP2(STOP_TIMER(CgProgramInfo_reloadIfRequired_reload_compile));

            START_TIMER(CgProgramInfo_reloadIfRequired_reload_load);
            cgGLLoadProgram(_myCgProgramID);
            STOP_TIMER(CgProgramInfo_reloadIfRequired_reload_load);
            assertCg(PLUS_FILE_LINE, _myContext);

            DBP2(START_TIMER(CgProgramInfo_reloadIfRequired_enableProfile));
            enableProfile();
            DBP2(STOP_TIMER(CgProgramInfo_reloadIfRequired_enableProfile));
            DBP2(START_TIMER(CgProgramInfo_reloadIfRequired_bindMaterialParams));
            bindMaterialParams(theMaterial);
            DBP2(STOP_TIMER(CgProgramInfo_reloadIfRequired_bindMaterialParams));
            DBP2(START_TIMER(CgProgramInfo_reloadIfRequired_enableTextures));
            enableTextures();
            DBP2(STOP_TIMER(CgProgramInfo_reloadIfRequired_enableTextures));
            assertCg(PLUS_FILE_LINE, _myContext);
        }

        return myReload;
    }

    void
    CgProgramInfo::bindBodyParams(
        const MaterialBase & theMaterial,
        const LightVector & theLightInstances,
        const Viewport & theViewport,
        const Body & theBody,
        const Camera & theCamera)
    {
        MAKE_GL_SCOPE_TIMER(CgProgramInfo_bindBodyParams);
        setCGGLParameters();

        std::vector<Vector3f> myPositionalLights;
        std::vector<Vector4f> myPositionalLightDiffuseColors;
        std::vector<Vector4f> myPositionalLightSpecularColors;

        std::vector<Vector3f> myDirectionalLights;
        std::vector<Vector4f> myDirectionalLightDiffuseColors;
        std::vector<Vector4f> myDirectionalLightSpecularColors;

        std::vector<Vector3f> mySpotLights;
        std::vector<Vector4f> mySpotLightDiffuseColors;
        std::vector<Vector4f> mySpotLightSpecularColors;
        std::vector<Vector3f> mySpotLightDirection;
        std::vector<float> mySpotLightCutoff;
        std::vector<float> mySpotLightExponent;
        std::vector<float> mySpotLightAttenuation;

        Vector4f myAmbientLightColor(0,0,0,1);

        // Collect light information
        for (unsigned i = 0; i < theLightInstances.size(); ++i) {
            LightPtr myLight = theLightInstances[i];
            if ( ! myLight->get<VisibleTag>()) {
                continue;
            }
            LightSourcePtr myLightSource = myLight->getLightSource();
            LightPropertiesFacadePtr myLightPropFacade = myLightSource->getChild<LightPropertiesTag>();
            switch (myLightSource->getType()) {
            case POSITIONAL :
                myPositionalLights.push_back(myLight->get<GlobalMatrixTag>().getTranslation());
                myPositionalLightDiffuseColors.push_back(myLightPropFacade->get<LightDiffuseTag>());
                myPositionalLightSpecularColors.push_back(myLightPropFacade->get<LightSpecularTag>());
                break;
            case DIRECTIONAL :
            {
                const Vector4f & myDirection = myLight->get<GlobalMatrixTag>().getRow(2);
                myDirectionalLights.push_back(Vector3f(myDirection[0],
                                                       myDirection[1],myDirection[2]));
                myDirectionalLightDiffuseColors.push_back(myLightPropFacade->get<LightDiffuseTag>());
                myDirectionalLightSpecularColors.push_back(myLightPropFacade->get<LightSpecularTag>());
                break;
            }
            case AMBIENT :
                myAmbientLightColor = myLightPropFacade->get<LightAmbientTag>();
                break;
            case SPOT:
            {
                const Matrix4f & myGlobalMatrix = myLight->get<GlobalMatrixTag>();
                Vector4f myDirection = myGlobalMatrix.getRow(2);
                myDirection.mult(-1.0);                        
                mySpotLightDirection.push_back(normalized(Vector3f(myDirection[0],myDirection[1],myDirection[2])));
                mySpotLights.push_back(myGlobalMatrix.getTranslation());
                mySpotLightDiffuseColors.push_back(myLightPropFacade->get<LightDiffuseTag>());
                mySpotLightSpecularColors.push_back(myLightPropFacade->get<LightSpecularTag>());
                mySpotLightCutoff.push_back(myLightPropFacade->get<CutOffTag>());
                mySpotLightExponent.push_back(myLightPropFacade->get<ExponentTag>());
                mySpotLightAttenuation.push_back(myLightPropFacade->get<AttenuationTag>());
                break;
            }
            default :
                AC_WARNING << "unknown light type " << myLightSource->get<IdTag>();
            }
        }


        // set parameter values
        for (CgProgramAutoParams::iterator myIter = _myAutoParams.begin();
             myIter != _myAutoParams.end(); ++myIter)
        {
            bool myParamValueFoundFlag = true;

            const CgProgramAutoParam & curParam = myIter->second;
            AC_TRACE << "setting parameter " << curParam._myName;

            switch (curParam._myID) {
            case CAMERA_POSITION :
                setCgVectorParameter(curParam, theCamera.get<GlobalMatrixTag>().getTranslation());
                break;
            case POSITIONAL_LIGHTS :
				{AC_TRACE << "setting POSITIONAL_LIGHTS to " << myPositionalLights;}
                setCgUnsizedArrayParameter(curParam, myPositionalLights);
                break;
            case POSITIONAL_LIGHTS_DIFFUSE_COLOR :
                setCgUnsizedArrayParameter(curParam, myPositionalLightDiffuseColors);
                break;
            case POSITIONAL_LIGHTS_SPECULAR_COLOR :
                setCgUnsizedArrayParameter(curParam, myPositionalLightSpecularColors);
                break;
            case DIRECTIONAL_LIGHTS :
				{AC_TRACE << "setting DIRECTIONAL_LIGHTS to " << myDirectionalLights;}
                setCgUnsizedArrayParameter(curParam, myDirectionalLights);
                break;
            case DIRECTIONAL_LIGHTS_DIFFUSE_COLOR :
                setCgUnsizedArrayParameter(curParam, myDirectionalLightDiffuseColors);
                break;
            case DIRECTIONAL_LIGHTS_SPECULAR_COLOR :
                setCgUnsizedArrayParameter(curParam, myDirectionalLightSpecularColors);
                break;
            case SPOT_LIGHTS :
				{AC_TRACE << "setting SPOT_LIGHTS to " << mySpotLights;}
                setCgUnsizedArrayParameter(curParam, mySpotLights);
                break;
            case SPOT_LIGHTS_DIFFUSE_COLOR :
                setCgUnsizedArrayParameter(curParam, mySpotLightDiffuseColors);
                break;
            case SPOT_LIGHTS_SPECULAR_COLOR :
                setCgUnsizedArrayParameter(curParam, mySpotLightSpecularColors);
                break;
            case SPOT_LIGHTS_EXPONENT :
                setCgUnsizedArrayParameter(curParam, mySpotLightExponent);
                break;
            case SPOT_LIGHTS_CUTOFF :
                setCgUnsizedArrayParameter(curParam, mySpotLightCutoff);
                break;
            case SPOT_LIGHTS_DIRECTION :
                setCgUnsizedArrayParameter(curParam, mySpotLightDirection);
                break;
            case SPOT_LIGHTS_ATTENUATION :
                setCgUnsizedArrayParameter(curParam, mySpotLightAttenuation);
                break;
            case AMBIENT_LIGHT_COLOR :
                setCgVectorParameter(curParam, myAmbientLightColor);
                break;
            case CAMERA_I:
                setCgMatrixParameter(curParam, theCamera.get<InverseGlobalMatrixTag>());
                break;
            case CAMERA_T:
            {
                Matrix4f myTransposedMatrix = theCamera.get<GlobalMatrixTag>();
                myTransposedMatrix.transpose();
                setCgMatrixParameter(curParam, myTransposedMatrix);
                break;
            }
            case VIEWPROJECTION:
            {
                Matrix4f myMatrix = theCamera.get<InverseGlobalMatrixTag>();
                //myMatrix.postMultiply(theViewport.get<ProjectionMatrixTag>()); XXX
                Matrix4f myProjectionMatrix;
                theCamera.get<FrustumTag>().getProjectionMatrix( myProjectionMatrix );
                myMatrix.postMultiply(myProjectionMatrix);
                setCgMatrixParameter(curParam, myMatrix);
                break;
            }
            case OBJECTWORLD:
                setCgMatrixParameter(curParam, theBody.get<GlobalMatrixTag>());
                break;
            case OBJECTWORLD_I:
                setCgMatrixParameter(curParam, theBody.get<InverseGlobalMatrixTag>());
                break;
            case OBJECTWORLD_T:
            {
                Matrix4f myTransposedMatrix = theBody.get<GlobalMatrixTag>();
                myTransposedMatrix.transpose();
                setCgMatrixParameter(curParam, myTransposedMatrix);
                break;
            }
            case OBJECTWORLD_IT:
            {
                Matrix4f myTransposedMatrix = theBody.get<InverseGlobalMatrixTag>();
                myTransposedMatrix.transpose();
                setCgMatrixParameter(curParam, myTransposedMatrix);
                break;
            }
            case TEXTURE_MATRICES:
            {
                int mySize = cgGetArraySize(curParam._myParameter, 0);
                for (unsigned i = 0; i < mySize; ++i) {
                    CGparameter myParam = cgGetArrayParameter(curParam._myParameter, i);
                    Matrix4f myTextureMatrix = theMaterial.getTextureUnit(i).get<TextureUnitMatrixTag>();
                    cgGLSetMatrixParameterfc(myParam, myTextureMatrix.getData());
                }
                break;
            }
            default:
                myParamValueFoundFlag = false;
                break;
            }
            if (!myParamValueFoundFlag) {
                AC_WARNING << "no value for cg auto parameter " << curParam._myName << endl;
            }
        }
    }

    void
    CgProgramInfo::bindOverlayParams() {
        setCGGLParameters();

        for (CgProgramAutoParams::iterator myIter = _myAutoParams.begin();
             myIter != _myAutoParams.end(); ++myIter)
        {
            CgProgramAutoParam curParam = myIter->second;
            AC_WARNING << "Cg parameter " << curParam._myName << " is not supported for overlays, yet.";
        }
    }

    void
    CgProgramInfo::bindMaterialParams(const MaterialBase & theMaterial) {
        AC_TRACE << "CgProgramInfo::bindMaterialParams shader filename=" << _myShader._myFilename << " entry=" << _myShader._myEntryFunction << " material=" << theMaterial.get<NameTag>();
        const MaterialPropertiesFacadePtr myPropFacade = theMaterial.getChild<MaterialPropertiesTag>();
        const Facade::PropertyMap & myProperties = myPropFacade->getProperties();
        for (unsigned i=0; i < _myMiscParams.size(); ++i) {
            Facade::PropertyMap::const_iterator myIter = myProperties.find(_myMiscParams[i]._myParamName);

            if (myIter != myProperties.end()) {

                if (myIter->second == 0){
                    AC_ERROR << "material property "
                             << _myMiscParams[i]._myParamName 
                             << " not defined!";
                    return;
                } 

                setCgMaterialParameter(_myMiscParams[i]._myParameter, *(myIter->second), 
                                       _myMiscParams[i]._myParamName, theMaterial);                
            }
        }
        for (unsigned i=0; i < _myTextureParams.size(); ++i) {
            Facade::PropertyMap::const_iterator myIter = myProperties.find(_myTextureParams[i]._myParamName);
            if (myIter != myProperties.end()) {

                setCgMaterialParameter(_myTextureParams[i]._myParameter, *(myIter->second), 
                                       _myTextureParams[i]._myParamName, theMaterial);                
            }
        }                        
    }

    void
    CgProgramInfo::setCgMaterialParameter(CGparameter & theCgParameter,
                                          const dom::Node & theNode,
                                          const std::string & thePropertyName,
                                          const MaterialBase & theMaterial)
    {


        AC_TRACE << "CgProgramInfo::setCgMaterialParameter: cgparam=" << theCgParameter << " node=" << theNode.nodeName() << " value=" << theNode.nodeValue() << " property=" << thePropertyName << " material=" << theMaterial.get<NameTag>() << " type=" << theNode.parentNode()->nodeName();
        TypeId myType;
        myType.fromString(theNode.parentNode()->nodeName());
//        switch(TypeId(asl::getEnumFromString(theNode.parentNode()->nodeName(), TypeIdStrings))) {
        switch(myType) {
        case FLOAT:
        {
            float myValue = theNode.nodeValueAs<float>();
            cgGLSetParameter1f(theCgParameter, myValue);
            break;
        }
        case VECTOR2F:
        {
            Vector2f myValueV = theNode.nodeValueAs<Vector2f>();
            float * myValue = myValueV.begin();
            cgGLSetParameter2fv(theCgParameter, myValue);
            break;
        }
        case VECTOR3F:
        {
            Vector3f myValueV = theNode.nodeValueAs<Vector3f>();
            float * myValue = myValueV.begin();
            cgGLSetParameter3fv(theCgParameter, myValue);
            break;
        }
        case VECTOR4F:
        {
            Vector4f myValueV = theNode.nodeValueAs<Vector4f>();
            float * myValue = myValueV.begin();
            cgGLSetParameter4fv(theCgParameter, myValue);
            break;
        }
        case VECTOR_OF_FLOAT:
        {
            VectorOfFloat myValueV = theNode.nodeValueAs<VectorOfFloat>();
            float * myValue = &myValueV[0];
            cgGLSetParameterArray1f(theCgParameter, 0, myValueV.size(), myValue);
            break;
        }
        case VECTOR_OF_VECTOR2F:
        {
            VectorOfVector2f myValueV = theNode.nodeValueAs<VectorOfVector2f>();
            float * myValue = myValueV.begin()->begin();
            cgGLSetParameterArray2f(theCgParameter, 0, myValueV.size(), myValue);
            break;
        }
        case VECTOR_OF_VECTOR4F:
        {
            VectorOfVector4f myValueV = theNode.nodeValueAs<VectorOfVector4f>();
            float * myValue = myValueV.begin()->begin();
            cgGLSetParameterArray4f(theCgParameter, 0, myValueV.size(), myValue);
            break;
        }
        case SAMPLER2D:
        case SAMPLER3D:
        case SAMPLERCUBE:
        {
            unsigned myTextureIndex = theNode.nodeValueAs<unsigned>();
            if (myTextureIndex < theMaterial.getTextureUnitCount()) {
                const TextureUnit & myTextureUnit = theMaterial.getTextureUnit(myTextureIndex);
                const TexturePtr & myTexture = myTextureUnit.getTexture();

                unsigned myTextureId = myTexture->getTextureId();
                AC_TRACE << "cgGLSetTextureParameter param=" << theCgParameter << " texid=" << myTextureId;
                cgGLSetTextureParameter( theCgParameter, myTextureId);
                DB(AC_TRACE << "cgGLSetTextureParameter: Texture index " << as_string(myTextureIndex)
                   << ", texid=" << myTextureId << ", property=" << thePropertyName
                   << " to parameter : "<< cgGetParameterName(theCgParameter) << endl);
            } else {
                throw ShaderException(string("Texture index ") + as_string(myTextureIndex) +
                                      " not found. Material id=" + theMaterial.get<IdTag>() + " name=" + theMaterial.get<NameTag>() + " has " + as_string(theMaterial.getTextureUnitCount()) + " texture(s)",
                                      "CgProgramInfo::setCgMaterialParameter()");
            }
            break;
        }
        default:
            throw ShaderException(string("Unknown CgParameter type in property '")+thePropertyName+"'",
                                  "CgProgramInfo::setCgMaterialParameter()");
        }

        assertCg(PLUS_FILE_LINE, 0);
    }

    void
    CgProgramInfo::setCgVectorParameter(const CgProgramAutoParam & theParam,
                                        const asl::Vector3f & theValue)
    {
        switch (theParam._myType) {
        case CG_FLOAT3:
            cgGLSetParameter3fv(theParam._myParameter, theValue.begin());
            break;
        case CG_FLOAT4:
        {
            Vector4f myValue4f(theValue[0], theValue[1], theValue[2], 1);
            cgGLSetParameter4fv(theParam._myParameter, myValue4f.begin());
        }
        break;
        default:
            throw RendererException ("Error in " + _myPathName + ": Parameter " + theParam._myName
                                     + " should be FLOAT3 or FLOAT4.", "CgProgramInfo::setCgVectorParameter()");
        }
        assertCg(string("setCgVectorParameter ") + theParam._myName, _myContext);
    }

    void
    CgProgramInfo::setCgVectorParameter(const CgProgramAutoParam & theParam,
                                        const asl::Vector4f & theValue)
    {
        switch (theParam._myType) {
        case CG_FLOAT3:
            cgGLSetParameter3fv(theParam._myParameter, theValue.begin());
            break;
        case CG_FLOAT4:
            cgGLSetParameter4fv(theParam._myParameter, theValue.begin());
            break;
        default:
            throw RendererException ("Error in " + _myPathName + ": Parameter " + theParam._myName
                                     + " should be FLOAT3 or FLOAT4.", "CgProgramInfo::setCgVectorParameter()");
        }
        assertCg(string("setCgVectorParameter ") + theParam._myName, _myContext);
    }

    void
    CgProgramInfo::setCgMatrixParameter(const CgProgramAutoParam & theParam,
                                        const asl::Matrix4f & theValue)
    {
        if (theParam._myType != CG_FLOAT4x4) {
            throw RendererException ("Error in " + _myPathName + ": Parameter " + theParam._myName
                                     + " should be FLOAT4x4.", "CgProgramInfo::setCgVectorParameter()");
        }
        cgGLSetMatrixParameterfc(theParam._myParameter, theValue.getData());
        assertCg(string("setCgMatrixParameter ") + theParam._myName, _myContext);
    }

    void
    CgProgramInfo::setCgArrayVector3fParameter(const CgProgramAutoParam & theParam,
                                               const vector<asl::Vector3f> & theValue)
    {
        if (theParam._myType == CG_ARRAY) {
            int mySize = cgGetArraySize(theParam._myParameter, 0);
            AC_TRACE << "setting array 3f size " << mySize;
            if (mySize) {
                cgGLSetParameterArray3f(theParam._myParameter, 0,
                                        mySize, theValue.begin()->begin());
            }
        } else {
            throw RendererException ("Error in " + _myPathName + ": Parameter " + theParam._myName
                                     + " should be FLOAT3[].", "CgProgramInfo::setCgArrayVector3fParameter()");
        }
        assertCg(string("setCgArrayVector3fParameter ") + theParam._myName, _myContext);
    }

    void
    CgProgramInfo::setCgArrayVector4fParameter(const CgProgramAutoParam & theParam,
                                               const vector<asl::Vector4f> & theValue)
    {
        if (theParam._myType == CG_ARRAY) {
            int mySize = cgGetArraySize(theParam._myParameter, 0);
            if (mySize) {
                cgGLSetParameterArray4f(theParam._myParameter, 0,
                                        mySize, theValue.begin()->begin());
            }
        } else {
            throw RendererException ("Error in " + _myPathName + ": Parameter " + theParam._myName
                                     + " should be FLOAT4[].", "CgProgramInfo::setCgArrayVector4fParameter()");
        }
        assertCg(string("setCgArrayVector4fParameter ") + theParam._myName, _myContext);
    }

    void
    CgProgramInfo::setCgUnsizedArrayParameter(const CgProgramAutoParam & theParam,
                                              const vector<float> & theValue)
    {
        int mySize = _myUnsizedArrayAutoParamSizes[theParam._myID];
        // int mySize = cgGetArraySize(theParam._myParameter, 0);
        if (mySize != theValue.size()) {
            AC_ERROR << "BUG 391: Cg Array " << theParam._myName << " expects " << mySize <<
                " elements, have " << theValue.size() << "(" << _myPathName << ")";
        }
        for(int i = 0; i < mySize; ++i) {
            CGparameter myElement = cgGetArrayParameter(theParam._myParameter, i);
            //AC_TRACE << "setting component " << i << " to " << theValue[i];
            if (i < theValue.size()) {
                cgSetParameter1f(myElement, theValue[i]);
            }
        }
        assertCg(string("setCgUnsizedArrayParameter ") + theParam._myName, _myContext);
    }

    void
    CgProgramInfo::setCgUnsizedArrayParameter(const CgProgramAutoParam & theParam,
                                              const vector<asl::Vector3f> & theValue)
    {
        int mySize = _myUnsizedArrayAutoParamSizes[theParam._myID];
        // int mySize = cgGetArraySize(theParam._myParameter, 0);
        if (mySize != theValue.size()) {
            AC_ERROR << "BUG 391: Cg Array " << theParam._myName << " expects " << mySize <<
                " elements, have " << theValue.size() << "(" << _myPathName << ")";
        }
        for(int i = 0; i < mySize; ++i) {
            CGparameter myElement = cgGetArrayParameter(theParam._myParameter, i);
            //AC_TRACE << "setting component " << i << " to " << theValue[i];
            if (i < theValue.size()) {
                cgSetParameter3f(myElement, theValue[i][0], theValue[i][1], theValue[i][2]);
            }
        }
        assertCg(string("setCgUnsizedArrayParameter ") + theParam._myName, _myContext);
    }

    void
    CgProgramInfo::setCgUnsizedArrayParameter(const CgProgramAutoParam & theParam,
                                              const vector<asl::Vector4f> & theValue)
    {
        int mySize = _myUnsizedArrayAutoParamSizes[theParam._myID];
        // int mySize = cgGetArraySize(theParam._myParameter, 0);
        if (mySize != theValue.size()) {
            AC_ERROR << "BUG 391: Cg Array " << theParam._myName << " expects " << mySize <<
                " elements, have " << theValue.size();
        }
        for(unsigned i = 0; i < mySize; ++i) {
            CGparameter myElement = cgGetArrayParameter(theParam._myParameter, i);
            //AC_TRACE << "setting component " << i << " to " << theValue[i];
            cgSetParameter4f(myElement, theValue[i][0], theValue[i][1], theValue[i][2], theValue[i][3]);
        }
        assertCg(string("setCgUnsizedArrayParameter ") + theParam._myName, _myContext);
    }

    void
    CgProgramInfo::bind() {
        //AC_TRACE << "CgProgramInfo::bind " << _myShader._myEntryFunction;
        CHECK_OGL_ERROR;
        DB(AC_TRACE << "binding " << _myShader._myEntryFunction << ":" <<_myCgProgramID << endl;)
            cgGLBindProgram(_myCgProgramID);
        assertCg(string("CgProgramInfo::bind() ") + _myPathName, _myContext);
    }

    void
    CgProgramInfo::enableTextures() {
        //AC_TRACE << "CgProgramInfo::enableTextures";
        for (unsigned i=0; i < _myTextureParams.size(); ++i) {

            GLenum myTexUnit = cgGLGetTextureEnum(_myTextureParams[i]._myParameter);
            glActiveTexture(myTexUnit);

            // AC_TRACE << "CgProgramInfo::enableTextures paramName=" << _myTextureParams[i]._myParamName
            //          << " param=" << _myTextureParams[i]._myParameter << " unit=" << hex << myTexUnit << dec;
            cgGLEnableTextureParameter(_myTextureParams[i]._myParameter);
            CHECK_OGL_ERROR;
        }
    }

    void
    CgProgramInfo::disableTextures() {
        //AC_TRACE << "CgProgramInfo::disableTextures";
        for (unsigned i=0; i < _myTextureParams.size(); ++i) {

            GLenum myTexUnit = cgGLGetTextureEnum(_myTextureParams[i]._myParameter);
            glActiveTexture(myTexUnit);

            //AC_TRACE << "CgProgramInfo::disableTextures paramName=" << _myTextureParams[i]._myParamName << " param=" << _myTextureParams[i]._myParameter << " unit=" << hex << myTexUnit << dec;
            cgGLDisableTextureParameter(_myTextureParams[i]._myParameter);
            CHECK_OGL_ERROR;
        }
    }

    CGprofile
    CgProgramInfo::asCgProfile(const ShaderDescription & theShader) {
        CGprofile myResult;
        switch (theShader._myProfile) {
        case ARBVP1 :
            myResult = CG_PROFILE_ARBVP1;
            break;
        case ARBFP1 :
            myResult = CG_PROFILE_ARBFP1;
            break;
        case GP4VP:
            myResult = CG_PROFILE_GPU_VP;
            break;
        case GP4FP:
            myResult = CG_PROFILE_GPU_FP;
            break;
        case  VP40:
            myResult = CG_PROFILE_VP40;
            break;
        case  FP40:
            myResult = CG_PROFILE_FP40;
            break;
        case  VP30:
            myResult = CG_PROFILE_VP30;
            break;
        case  FP30:
            myResult = CG_PROFILE_FP30;
            break;
        case  VP20:
            myResult = CG_PROFILE_VP20;
            break;
        case  FP20:
            myResult = CG_PROFILE_FP20;
            break;
        case GLSLV:
            myResult = CG_PROFILE_GLSLV;
            break;
        case GLSLF:
            myResult = CG_PROFILE_GLSLF;
            break;
        default:
            throw RendererException(string("Unknown shaderprofile : ") +
                                    getStringFromEnum(theShader._myProfile, ShaderProfileStrings),
                                    PLUS_FILE_LINE);
        };
        return myResult;
    }
}
