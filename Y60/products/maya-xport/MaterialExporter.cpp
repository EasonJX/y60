//=============================================================================
// Copyright (C) 2000-2002, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: MaterialExporter.cpp,v $
//   $Author: jens $
//   $Revision: 1.81 $
//   $Date: 2005/04/27 16:29:55 $
//
//  Description: This class implements a polygon exporter plugin for maya.
//
//=============================================================================

#include "MayaHelpers.h"
#include "MaterialExporter.h"
#include "ExportExceptions.h"

#define DEBUG_LEVEL 0

#include <y60/MaterialTypes.h>
#include <y60/DataTypes.h>
#include <y60/SceneBuilder.h>
#include <y60/ShapeBuilder.h>
#include <y60/MaterialBuilder.h>
#include <y60/NodeValueNames.h>
#include <y60/PropertyNames.h>
#include <y60/property_functions.h>

#include <asl/string_functions.h>
#include <asl/numeric_functions.h>
#include <asl/Dashboard.h>
#include <asl/Matrix4.h>

#include <maya/MGlobal.h>
#include <maya/MObject.h>
#include <maya/MColor.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnAttribute.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MFnPhongShader.h>
#include <maya/MFnPhongEShader.h>
//#include <maya/MFn.h>
#include <maya/MFnBlinnShader.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTransform.h>
#include <maya/MFnMatrixData.h>
#include <maya/MQuaternion.h>
#include <maya/MMatrix.h>
#include <maya/MBoundingBox.h>
#include <maya/MEulerRotation.h>

#include <iostream>
#include <algorithm>
#include <cctype> // needed for std::toupper

#define DB(x) // x;
#define DB_Y(x)  x

using namespace std;
using namespace asl;
using namespace y60;

static const MayaBlendMode ourDefaultBlendMode = MAYA_BLEND_MULTIPLY;

MObject
MaterialExporter::findShader(MObject & theSetNode) {
    MFnDependencyNode myFnNode(theSetNode);
    MPlug myShaderPlug = myFnNode.findPlug("surfaceShader");

    if (myShaderPlug.isNull()) {
        throw ExportException("Could not find shader plug", "MaterialExporter::findShader()");
    }

    MPlugArray myConnectedPlugs;

    // Get all the plugs that are connected as the destination of this
    // surfaceShader plug so we can find the surface shaderNode
    MStatus myStatus;
    myShaderPlug.connectedTo(myConnectedPlugs, true, false, &myStatus);
    if (myStatus == MStatus::kFailure) {
        throw ExportException("No connected plug found", "MaterialExporter::findShader()");
    }

    if (myConnectedPlugs.length() != 1) {
        throw ExportException("Error: Shader has more than one or no connected plugs",
                              "MaterialExporter::findShader()");
    }

    return myConnectedPlugs[0].node();
}

MPlug
MaterialExporter::getPlug(const MObject & theShaderNode, const char * theName) {
    // Get the color plug
    MStatus myStatus;
    MPlug myColorPlug = MFnDependencyNode(theShaderNode).findPlug(theName, &myStatus);
    if (myStatus == MS::kFailure) {
        throw ExportException(std::string("No plug found with name'")+theName+"'", "MaterialExporter::getPlug()");
    }

    return myColorPlug;
}


void
MaterialExporter::exportFileTexture(const MFnMesh * theMesh, MObject & theTextureNode,
                                    y60::MaterialBuilder & theBuilder,
                                    y60::SceneBuilder & theSceneBuilder,
                                    asl::Vector4f theColorScale,
                                    asl::Vector4f theColorBias,
                                    const MayaBlendMode theBlendMode,
                                    const char* theColorGainPropertyName,
                                    float theColorGainAlpha)
{
    MStatus myStatus;

    AC_DEBUG << "textureNode name=" << MFnDependencyNode(theTextureNode).name().asChar();
    //dumpAttributes(theTextureNode);

    std::string myMaterialName = theBuilder.getName();

    // texture mapping
    string myTextureMapping = getTextureMappingType(theTextureNode);
    std::string myTextureName(MFnDependencyNode(theTextureNode).name().asChar());

    MString myFileName("");
    if (myTextureMapping == y60::TEXCOORD_UV_MAP) {
        MPlug myFilenamePlug = MFnDependencyNode(theTextureNode).findPlug("fileTextureName");
        myFilenamePlug.getValue(myFileName);
        _myMaterialUVMappedTexturesMap[myMaterialName].push_back(myTextureName);
    } else { // generative mapping
        MPlug myImagePlug = MFnDependencyNode(theTextureNode).findPlug("image");
        MObject myImageNode;
        getConnectedNode(myImagePlug, myImageNode);
        MPlug myFilenamePlug = MFnDependencyNode(myImageNode).findPlug("fileTextureName");
        myFilenamePlug.getValue(myFileName);
    }

    std::string myStrippedFileName = myFileName.asChar();
    stripBaseDir(myStrippedFileName);
    AC_DEBUG << "stripped filename=" << myStrippedFileName;

    // color gain
    float r = 1.0f, g = 1.0f, b = 1.0f;
    if (theColorGainPropertyName) {
        MPlug myColorGainPlug = MFnDependencyNode(theTextureNode).findPlug("colorGainR");
        myColorGainPlug.getValue(r);
        myColorGainPlug = MFnDependencyNode(theTextureNode).findPlug("colorGainG");
        myColorGainPlug.getValue(g);
        myColorGainPlug = MFnDependencyNode(theTextureNode).findPlug("colorGainB");
        myColorGainPlug.getValue(b);
        y60::setPropertyValue<asl::Vector4f>(theBuilder.getNode(),
                "vector4f", theColorGainPropertyName,
                asl::Vector4f(r, g, b, float(theColorGainAlpha)));
    }

    // get custom attributes
    bool myCreateMipmapsFlag = true;
    getCustomAttribute(theTextureNode, "ac_mipmaps", myCreateMipmapsFlag);

    std::string myApplyMode = y60::TEXTURE_APPLY_MODULATE;
    std::string myUsage     = y60::TEXTURE_USAGE_PAINT;
    switch (theBlendMode) {
        case MAYA_BLEND_NONE:
            myApplyMode = y60::TEXTURE_APPLY_REPLACE;
            break;
        case MAYA_BLEND_OVER:
            myApplyMode = y60::TEXTURE_APPLY_DECAL;
            break;
        case MAYA_BLEND_MULTIPLY:
            myApplyMode = y60::TEXTURE_APPLY_MODULATE;
            break;
        case MAYA_BLEND_ADD:
            myApplyMode = y60::TEXTURE_APPLY_ADD;
            break;
        default:
            displayWarning(string("Blendmode: ") + getStringFromEnum(theBlendMode, MayaBlendModesString) + " not yet implemented.");
    }

    string myImageId = theBuilder.createImage(theSceneBuilder,
        MFnDependencyNode(theTextureNode).name().asChar(),
        myStrippedFileName, myUsage, myCreateMipmapsFlag,
        theColorScale, theColorBias, SINGLE,"");

    // wrap
    std::string myWrapMode = y60::TEXTURE_WRAP_REPEAT;

    bool myWrapU = false;
    MPlug myWarpUPlug = MFnDependencyNode(theTextureNode).findPlug("wrapU");
    if (myWarpUPlug.getValue(myWrapU) != MStatus::kSuccess) {
        AC_WARNING << "No wrapU";
    }

    bool myWrapV = false;
    MPlug myWarpVPlug = MFnDependencyNode(theTextureNode).findPlug("wrapV");
    if (myWarpVPlug.getValue(myWrapV) != MStatus::kSuccess) {
        AC_WARNING << "No wrapV";
    }
    if (!myWrapU && !myWrapV) {
        myWrapMode = TEXTURE_WRAP_CLAMP;
    }

    // texture matrix
    asl::Matrix4f myTextureMatrix;
    myTextureMatrix.makeIdentity();

    MObject myPlacementMatrix;
    MPlug myPlacementMatrixPlug = MFnDependencyNode(theTextureNode).findPlug("placementMatrix");
    myPlacementMatrixPlug.getValue(myPlacementMatrix);
    MFnMatrixData myMatrixData(myPlacementMatrix, &myStatus);
    if (myStatus == MStatus::kSuccess) {

        // fetch placement matrix
        MTransformationMatrix myTransform = myMatrixData.transformation();
        MMatrix myMayaMatrix = myTransform.asMatrix(); 

        asl::Matrix4f myMatrix;
        myMatrix.assign(
            float(myMayaMatrix[0][0]), float(myMayaMatrix[0][1]), float(myMayaMatrix[0][2]), float(myMayaMatrix[0][3]),
            float(myMayaMatrix[1][0]), float(myMayaMatrix[1][1]), float(myMayaMatrix[1][2]), float(myMayaMatrix[1][3]),
            float(myMayaMatrix[2][0]), float(myMayaMatrix[2][1]), float(myMayaMatrix[2][2]), float(myMayaMatrix[2][3]),
            float(myMayaMatrix[3][0]), float(myMayaMatrix[3][1]), float(myMayaMatrix[3][2]), float(myMayaMatrix[3][3]));

        if (myTextureMapping != TEXCOORD_UV_MAP) {

            /*
             * STATE-OF-THE-UNION:
             * - right now rotations don't work with scaling
             * - for some friggin' reason the scaling values we get
             *   from the matrix are same for X&Y
             * - probably the matrix from Maya already contains all this...
             */

            double mayaScale[3];
            myTransform.getScale(mayaScale, MSpace::kWorld);
            cout << "+++ maya scale=" << mayaScale[0] << "," << mayaScale[1] << "," << mayaScale[2] << endl;

            MVector mayaTrans = myTransform.translation(MSpace::kWorld);
            cout << "+++ maya trans=" << mayaTrans[0] << "," << mayaTrans[1] << "," << mayaTrans[2] << endl;

            MEulerRotation mayaEuler = myTransform.eulerRotation();
            cout << "+++ maya euler=" << mayaEuler[0] << "," << mayaEuler[1] << "," << mayaEuler[2] << endl;

            /*
             * Calculate texgen matrix
             */
            cout << "***" << endl;
            cout << __DATE__ << " " << __TIME__ << endl;
            cout << "mtx=" << myMatrix << endl;
#if 0
            asl::Matrix4f myMtxInv = myMatrix;
            myMtxInv.invert();
            cout << "inv=" << myMtxInv << endl;
#endif

            // object bounding-box for size normalization
            MBoundingBox myBBox = theMesh->boundingBox(&myStatus);
            if (!myStatus) {
                AC_ERROR << "Unable to get bounding box";
                throw ExportException("Unable to get bounding box.", PLUS_FILE_LINE);
            }
            //cout << "bbox="  << myBBox.width() << "x" << myBBox.height() << "x" << myBBox.depth() << endl;
            MPoint myBBoxSize(myBBox.width(), myBBox.height(), myBBox.depth());
            //convertToMeter(myBBoxSize);
            cout << "bbox size=" << myBBoxSize.x << "," << myBBoxSize.y << "," << myBBoxSize.z << endl;

            asl::Matrix4f myNormalizationMatrix;
            myNormalizationMatrix.makeIdentity();
            myNormalizationMatrix.scale(asl::Vector3f(1.0f, -1.0f, 1.0f));
            myNormalizationMatrix.translate(asl::Vector3f(0.5f, 0.5f, 0.5f));

            // fetch parameters from matrix
            asl::Vector3f myScale, myShear, myRotation, myTranslation;
            myMatrix.decompose(myScale, myShear, myRotation, myTranslation);

            cout << "scale=" << myScale << endl;
            myScale[0] *= myBBoxSize.x * 0.5f;
            myScale[1] *= myBBoxSize.y * 0.5f;
            myScale[2] *= myBBoxSize.z * 0.5f;
            cout << "scale=" << myScale << endl;

            cout << "translation=" << myTranslation << endl;
            myTranslation[0] /= myScale[0] * 2.0f;
            myTranslation[1] /= myScale[1] * 2.0f;
            myTranslation[2] /= myScale[2] * 2.0f;
            cout << "translation=" << myTranslation << endl;
 
            cout << "euler=" << myRotation << endl;
            cout << "***" << endl;

            // texture scaling
            asl::Matrix4f myScaleMatrix;
            myScaleMatrix.makeScaling(myScale);
            //cout << "scale matrix=" << myScaleMatrix << endl;

            // texture translation
            asl::Matrix4f myPosMatrix;
            myPosMatrix.makeTranslating(myTranslation);
            //cout << "translation matrix=" << myPosMatrix << endl;

            // texture rotation
            asl::Matrix4f myRotMatrix;
            myRotMatrix.makeIdentity();
            //myRotMatrix.translate(asl::Vector3f(0.5f, 0.5f, 0.5f));
            //TODO switch over rotate-order
            myRotMatrix.rotateX(myRotation[0]);
            myRotMatrix.rotateY(myRotation[1]);
            myRotMatrix.rotateZ(myRotation[2]);
            //myRotMatrix.translate(asl::Vector3f(-0.5f, -0.5f, -0.5f));
            //cout << "rotation matrix=" << myRotMatrix << endl;

            // apply
            myMatrix.makeIdentity();
#if 0
            myMatrix.postMultiply(myScaleMatrix);
            myMatrix.postMultiply(myPosMatrix);
            myMatrix.postMultiply(myRotMatrix);
#else
            myMatrix.postMultiply(myRotMatrix);
            myMatrix.postMultiply(myPosMatrix);
            myMatrix.postMultiply(myScaleMatrix);
#endif
            cout << "pre-norm=" << myMatrix << endl;
            myMatrix.postMultiply(myNormalizationMatrix);
            cout << "final=" << myMatrix << endl;

            VectorOfVector4f myTexGenParams;
            myTexGenParams.push_back(myMatrix.getColumn(0));
            myTexGenParams.push_back(myMatrix.getColumn(1));
            myTexGenParams.push_back(myMatrix.getColumn(2));
            myTexGenParams.push_back(myMatrix.getColumn(3));
            setPropertyValue<VectorOfVector4f>(theBuilder.getNode(),
                    "vectorofvector4f", "texgenparam0", myTexGenParams);

            myWrapMode = TEXTURE_WRAP_REPEAT;
        } else {
            /*
             * Use texture placement matrix
             */
            myTextureMatrix = myMatrix;
       }
    } else {
        /*
         * No texture placement matrix, calculate repeat
         */

        // repeat
        double myRepeatU = 1;
        MPlug myRepeatUPlug = MFnDependencyNode(theTextureNode).findPlug("repeatU");
        myRepeatUPlug.getValue(myRepeatU);

        double myRepeatV = 1;
        MPlug myRepeatVPlug = MFnDependencyNode(theTextureNode).findPlug("repeatV");
        myRepeatVPlug.getValue(myRepeatV);

        myTextureMatrix.makeScaling(asl::Vector3f(float(myRepeatU), float(myRepeatV), 1));
    }
    cout << "texture matrix=" << myTextureMatrix << endl;

    theBuilder.createTextureNode(myImageId, myApplyMode, myUsage, myWrapMode,
            myTextureMapping, myTextureMatrix, 100, false, 50);
}

string 
MaterialExporter::getTextureMappingType(const MObject & theTextureNode) {
    string myResult;
    double myProjectionType = 0;
    MPlug myProjectionTypePlug = MFnDependencyNode(theTextureNode).findPlug("projType");
    myProjectionTypePlug.getValue(myProjectionType);
    unsigned myMappingIndex = unsigned(myProjectionType);
    if (myMappingIndex < 0 || myMappingIndex > 8 ) {
        throw ExportException(string("Sorry, mapping mode not supported: ") + asl::as_string(myMappingIndex), PLUS_FILE_LINE);
    }
 
    /*
     * Note:
     * - Mayas concentric mode is not supported, instead a spatial mode is authored.
     *   Will fail in Y60!!!
     * - PERSPECTIVE mode will be mapped to FRONTAL.
     */
    static const string myTexMapping[] = {
        TEXCOORD_UV_MAP, TEXCOORD_PLANAR, TEXCOORD_SPHERICAL, TEXCOORD_CYLINDRICAL,
        TEXCOORD_SPHERICAL, TEXCOORD_CUBE, TEXCOORD_CUBE, TEXCOORD_SPATIAL,
        TEXCOORD_FRONTAL
    };

    return myTexMapping[myMappingIndex];
}

void
MaterialExporter::exportBumpTexture(const MObject & theBumpNode,
                                    y60::MaterialBuilder & theBuilder,
                                    y60::SceneBuilder & theSceneBuilder)
{
    DB_Y(cerr << "MaterialExporter::exportBumpTexture() - name: " << MFnDependencyNode(theBumpNode).name().asChar() << endl);
    DB(dumpAttributes(theBumpNode));
    MPlug myBumpPlug = MFnDependencyNode(theBumpNode).findPlug("bumpValue");
    MObject myBumpFileNode;
    getConnectedNode(myBumpPlug, myBumpFileNode);

    DB(AC_TRACE << "MaterialExporter::exportBumpTexture(): myBumpFileNode:" <<endl);
    DB(dumpAttributes(myBumpFileNode));

    MString myFileName("");
    MPlug myFilenamePlug = MFnDependencyNode(myBumpFileNode).findPlug("fileTextureName");
    myFilenamePlug.getValue(myFileName);

    std::string myStrippedFileName = myFileName.asChar();
    stripBaseDir(myStrippedFileName);

    std::string myApplyMode = y60::TEXTURE_APPLY_DECAL;
    std::string myUsage     = y60::TEXTURE_USAGE_BUMP;
    std::string myWrapMode  = y60::TEXTURE_WRAP_REPEAT;

    string myImageId = theBuilder.createImage(theSceneBuilder,
        MFnDependencyNode(theBumpNode).name().asChar(),
        myStrippedFileName, myUsage, false, asl::Vector4f(1.0f,1.0f,1.0f,1.0f),
        asl::Vector4f(0.0f,0.0f,0.0f,0.0f), SINGLE, "");

    theBuilder.createTextureNode(myImageId, myApplyMode, myUsage, myWrapMode, TEXCOORD_UV_MAP, asl::Matrix4f::Identity(), 100, false, 0);
    theBuilder.needTextureFallback(true);
    std::string myMaterialName = theBuilder.getName();
    std::string myTextureName(MFnDependencyNode(theBumpNode).name().asChar());
    _myMaterialUVMappedTexturesMap[myMaterialName].push_back(myTextureName);

    DB(AC_TRACE << "MaterialExporter::exportBumpTexture() - Exporting texture: " << endl << myFileName.asChar());
}

void
MaterialExporter::exportEnvCubeTexture(const MObject & theShaderNode,
                                       const MObject & theEnvCubeNode,
                                    y60::MaterialBuilder & theBuilder,
                                    y60::SceneBuilder & theSceneBuilder)
{
    DB(AC_TRACE << "MaterialExporter::exportEnvCubeTexture() - Export texture object: " <<
          MFnDependencyNode(theEnvCubeNode).name().asChar());
    DB(dumpAttributes(theEnvCubeNode));

    MStatus myStatus;

    std::string frontFileName  = getStrippedTextureFilename(getPlug(theEnvCubeNode,"front"));
    std::string rightFileName  = getStrippedTextureFilename(getPlug(theEnvCubeNode,"right"));
    std::string backFileName   = getStrippedTextureFilename(getPlug(theEnvCubeNode,"back"));
    std::string leftFileName   = getStrippedTextureFilename(getPlug(theEnvCubeNode,"left"));
    std::string topFileName    = getStrippedTextureFilename(getPlug(theEnvCubeNode,"top"));
    std::string bottomFileName = getStrippedTextureFilename(getPlug(theEnvCubeNode,"bottom"));

    std::string myApplyMode = y60::TEXTURE_APPLY_MODULATE;

    float myReflectivity = MFnPhongShader(theShaderNode).reflectivity(& myStatus);
    asl::Vector4f myColorScale = asl::Vector4f(1, 1, 1, myReflectivity);
    
    MColor mySpecularColor = MFnReflectShader(theShaderNode).specularColor(& myStatus);
    
    theBuilder.appendCubemap(theSceneBuilder,
            MFnDependencyNode(theEnvCubeNode).name().asChar(),
            frontFileName, rightFileName, backFileName,
            leftFileName, topFileName, bottomFileName,
            myApplyMode, myColorScale);

    //NOTE: specular colors are added to the diffuse colors. 
    //      no alpha value support for specular colors therefore means 
    //      setting the specular alpha to zero.
    asl::Vector4f myY60SpecColor(mySpecularColor.r, mySpecularColor.g, mySpecularColor.b, 0); //mySpecularColor.a
    setPropertyValue<asl::Vector4f>(theBuilder.getNode(),
                    "vector4f", y60::SPECULAR_PROPERTY, myY60SpecColor);
/*
    float myShininess = MFnPhongShader(theShaderNode).cosPower(& myStatus);
    // Convert to 0 - 128 range of OpenGL
    myShininess = (myShininess / 100) * 128;
    setPropertyValue<float>(theBuilder.getNode(), "float", y60::SHININESS_PROPERTY, myShininess);
*/                      
}

void
MaterialExporter::exportLayeredTexture(const MFnMesh * theMesh, const MObject & theMultiTextureNode,
                                       y60::MaterialBuilder & theBuilder,
                                       y60::SceneBuilder & theSceneBuilder,
                                       const char* theColorGainPropertyName,
                                       float theColorGainAlpha)
{
    MStatus myStatus;
    MFnDependencyNode myDependencyNode(theMultiTextureNode);
    MPlug myInputPlug = myDependencyNode.findPlug("inputs", & myStatus);

    if (!myStatus) {
        throw ExportException("Could not find inputs plug", "MaterialExporter::exportLayeredTexture()");
    }

    DB(AC_TRACE << "MaterialExporter::exportLayeredTexture() - Number of layers: " <<
          myInputPlug.numElements());
    DB(dumpAttributes(theMultiTextureNode));

    // Iterate over all texture layers
    // Maya shades the first texture last, therefore we reverse the order
    for (int i = myInputPlug.numElements() - 1; i >= 0; --i) {
        // Export alpha and blend mode for the texture
        float myAlpha = 1.0;
        MPlug myAlphaPlug;
        getChildPlugByName("alpha", myInputPlug[i], theMultiTextureNode, myAlphaPlug);

        MObject myAlphaChannelNode;
        MStatus myStatus;
        MPlugArray myPlugArray;
        myAlphaPlug.connectedTo(myPlugArray, true, false, & myStatus);
        unsigned myLength = myPlugArray.length();
        if (myLength <= 0) {
            myAlphaPlug.getValue(myAlpha);
        }

        int myBlendModeValue = 0;
        MPlug myBlendPlug;
        getChildPlugByName("blendMode", myInputPlug[i], theMultiTextureNode, myBlendPlug);
        myBlendPlug.getValue(myBlendModeValue);

        MayaBlendMode myBlendMode = MayaBlendMode(myBlendModeValue);

        DB(AC_TRACE << "MaterialExporter::exportLayeredTexture() - alpha: " << myAlpha << ", blendmode: "
            << getStringFromEnum(myBlendMode, MayaBlendModesString));

        // Find connected texture node
        MPlug myColorPlug;
        MObject myTextureNode;
        getChildPlugByName("color", myInputPlug[i], theMultiTextureNode, myColorPlug);
        getConnectedNode(myColorPlug, myTextureNode);

        asl::Vector4f myColorScale;
        asl::Vector4f myColorBias;

        if (i == myInputPlug.numElements() - 1) {
            // Only the color gain (material color) for the bottom most texture is supported by the renderer.
            // To blend in the color correctly we assign multiply to the bottom most texture by default.
            myBlendMode = MAYA_BLEND_MULTIPLY;
            calcColorScaleAndBiasFromAlpha(myAlpha, myBlendMode, myColorScale, myColorBias);
            exportFileTexture(theMesh, myTextureNode, theBuilder, theSceneBuilder,
                    myColorScale, myColorBias, myBlendMode,
                    theColorGainPropertyName, theColorGainAlpha);
        } else {
            calcColorScaleAndBiasFromAlpha(myAlpha, myBlendMode, myColorScale, myColorBias);
            exportFileTexture(theMesh, myTextureNode, theBuilder, theSceneBuilder,
                    myColorScale, myColorBias, myBlendMode,
                    0, theColorGainAlpha);
        }
    }
}

bool
MaterialExporter::exportTextures(const MFnMesh * theMesh, const MObject & theShaderNode,
                                 y60::MaterialBuilder & theBuilder,
                                 y60::SceneBuilder & theSceneBuilder,
                                 const std::string & thePlugName,
                                 const char* theColorGainPropertyName,
                                 float theColorGainAlpha)
{
    try {
        return exportMaps(theMesh, theShaderNode,theBuilder, theSceneBuilder,
                thePlugName.c_str(), theColorGainPropertyName, theColorGainAlpha);
    } catch (const ExportException & ex) {
        MGlobal::displayError(MString((std::string("Can not export Texture: ")
                                      + ex.what() + " at " + ex.where()).c_str()));
        return false;
    }
}
bool
MaterialExporter::exportBumpMaps(const MFnMesh * theMesh, const MObject & theShaderNode,
                                 y60::MaterialBuilder & theBuilder,
                                 y60::SceneBuilder & theSceneBuilder,
                                 float theColorGainAlpha)
{
    try {
        return exportMaps(theMesh, theShaderNode,theBuilder, theSceneBuilder, "normalCamera", y60::DIFFUSE_PROPERTY, theColorGainAlpha);
    } catch (const ExportException & ex) {
        MGlobal::displayError(MString((std::string("Can not export Texture: ")
                                      + ex.what() + " at " + ex.where()).c_str()));
        return false;
    }
}


bool
MaterialExporter::exportMaps(const MFnMesh * theMesh, const MObject & theShaderNode,
                             y60::MaterialBuilder & theBuilder,
                             y60::SceneBuilder & theSceneBuilder,
                             const char * thePlugName,
                             const char* theColorGainPropertyName,
                             float theColorGainAlpha)
{
    bool hasTextures = false;
    MStatus myStatus;
    DB(AC_TRACE << "MaterialExporter::exportMaps("<<thePlugName<<")");

    DB(dumpAttributes(theShaderNode));
    MPlug myPlug = getPlug(theShaderNode,thePlugName);

    MPlugArray myPlugArray;
    myPlug.connectedTo(myPlugArray, true, false, & myStatus);
    if (myStatus == MS::kFailure) {
        throw ExportException("Could not get destination plug array",
                              "MaterialExporter::getTexture()");
    }

    if (myPlugArray.length() > 1) {
        throw ExportException("Unsupported type of color plug.",
                              "MaterialExporter::getTexture()");
    } else if (myPlugArray.length() == 1) {
        hasTextures = true;

        MObject myTextureNode(myPlugArray[0].node());
        MFn::Type myTextureType = myTextureNode.apiType();
        switch (myTextureType) {
            case MFn::kLayeredTexture:
                exportLayeredTexture(theMesh, myTextureNode, theBuilder, theSceneBuilder,
                                     theColorGainPropertyName, theColorGainAlpha);
                break;
            case MFn::kProjection:
            case MFn::kFileTexture:
                exportFileTexture(theMesh, myTextureNode, theBuilder, theSceneBuilder,
                                  asl::Vector4f(1.0f,1.0f,1.0f,1.0f), asl::Vector4f(0.0f,0.0f,0.0f,0.0f),
                                  ourDefaultBlendMode, theColorGainPropertyName,
                                  theColorGainAlpha);
                break;
            case MFn::kBump:
                exportBumpTexture(myTextureNode, theBuilder, theSceneBuilder);
                break;
            case MFn::kEnvCube:
                exportEnvCubeTexture(theShaderNode, myTextureNode, theBuilder, theSceneBuilder);
                break;
            default:
                throw ExportException(std::string("Unsupported type of map plug: ") +
                        myPlugArray[0].node().apiTypeStr(),
                        "MaterialExporter::exportMaps()");
        }

    }
    DB(AC_TRACE << "MaterialExporter::exportMaps("<<thePlugName<<") ready");

    return hasTextures;
}
void
MaterialExporter::exportUnlitFeatures(const MFnMesh * theMesh, const MObject & theShaderNode,
                                     y60::MaterialBuilder & theBuilder,
                                     y60::SceneBuilder & theSceneBuilder)
{
    DB(AC_TRACE << "MaterialExporter::exportUnliFeatures()");
    MStatus myStatus;

    try {
        // If no textures were found, we can export the color value instead
        MPlug myTransparencyPlug = MFnDependencyNode(theShaderNode).findPlug("outTransparencyR");
        float myColorGainAlpha;
        myTransparencyPlug.getValue(myColorGainAlpha);
        myColorGainAlpha = 1.0f - myColorGainAlpha;
        if ( ! exportTextures(theMesh, theShaderNode, theBuilder, theSceneBuilder, "outColor", y60::SURFACE_COLOR_PROPERTY, myColorGainAlpha)) {
            DB(dumpAttributes(theShaderNode));
            float r, g, b;
            MPlug myColorPlug = MFnDependencyNode(theShaderNode).findPlug("outColorR");
            myColorPlug.getValue(r);
            myColorPlug = MFnDependencyNode(theShaderNode).findPlug("outColorG");
            myColorPlug.getValue(g);
            myColorPlug = MFnDependencyNode(theShaderNode).findPlug("outColorB");
            myColorPlug.getValue(b);
            setPropertyValue<asl::Vector4f>(theBuilder.getNode(),
                    "vector4f",
                    y60::SURFACE_COLOR_PROPERTY, asl::Vector4f(r, g, b, float(myColorGainAlpha)));
        }
    } catch(asl::Exception & ex) {
        ex; // avoid unreferenced variable warning
        DB(AC_TRACE << ex);
        throw;
    }
}

void
MaterialExporter::exportLambertFeatures(const MFnMesh * theMesh, const MObject & theShaderNode,
                                        y60::MaterialBuilder & theBuilder,
                                        y60::SceneBuilder & theSceneBuilder)
{
    DB(AC_TRACE << "MaterialExporter::exportLambertFeatures()");
    MStatus myStatus;

    try {
        // If no textures were found, we can export the diffuse color value instead
        MColor myTransparency = MFnLambertShader(theShaderNode).transparency(& myStatus);
        DB(AC_TRACE << "myTransp = "<< asl::Vector4f(myTransparency.r, myTransparency.g, myTransparency.b, myTransparency.a) << endl);
       if ( ! exportTextures(theMesh, theShaderNode, theBuilder, theSceneBuilder, "color",
            y60::DIFFUSE_PROPERTY, 1.0f - myTransparency.r ))
       {
            MColor myColor = MFnLambertShader(theShaderNode).color(& myStatus);
            if (myStatus == MStatus::kFailure) {
                throw ExportException("Could not get color from node",
                        "MaterialExporter::exportLambertFeatures");
            }

            DB(AC_TRACE << "myColor  = "<< asl::Vector4f(myColor.r, myColor.g, myColor.b, myColor.a) << endl);

            setPropertyValue<asl::Vector4f>(theBuilder.getNode(),
                    "vector4f",
                    y60::DIFFUSE_PROPERTY,
                    asl::Vector4f(myColor.r, myColor.g, myColor.b, float(1.0f - myTransparency.r)));
        }
        exportBumpMaps(theMesh, theShaderNode, theBuilder, theSceneBuilder, 1.0f - myTransparency.r);
    } catch(asl::Exception & ex) {
        ex; // avoid unreferenced variable warning
        DB(AC_TRACE << ex);
        throw;
    }

    MColor myAmbientColor = MFnLambertShader(theShaderNode).ambientColor(& myStatus);
    if (myStatus == MStatus::kFailure) {
        throw ExportException("Could not get color from node",
                "MaterialExporter::exportLambertFeatures");
    }
    //setPropertyValue<asl::Vector4f>(theBuilder.getNode(),"vector4f", y60::DIFFUSE_PROPERTY, asl::Vector4f(1.0, 1.0, 1.0, 1.0));
    setPropertyValue<asl::Vector4f>(theBuilder.getNode(), "vector4f", y60::AMBIENT_PROPERTY,
            asl::Vector4f(myAmbientColor.r, myAmbientColor.g, myAmbientColor.b, myAmbientColor.a));
}

void
MaterialExporter::exportReflectiveFeatures(const MFnMesh * theMesh, const MObject & theShaderNode,
                                        y60::MaterialBuilder & theBuilder,
                                        y60::SceneBuilder & theSceneBuilder)
{
    DB(AC_TRACE << "MaterialExporter::exportReflectiveFeatures()");
    MStatus myStatus;
    if ( ! exportTextures(theMesh, theShaderNode, theBuilder, theSceneBuilder, "reflectedColor", y60::DIFFUSE_PROPERTY, 1.0)) {
        MColor mySpecularColor = MFnReflectShader(theShaderNode).specularColor(& myStatus);
        if (myStatus == MStatus::kFailure) {
            throw ExportException("Could not get specular color from node",
                                "MaterialExporter::exportReflectiveFeatures");
        }

        setPropertyValue<asl::Vector4f>(theBuilder.getNode(), "vector4f", y60::SPECULAR_PROPERTY,
            asl::Vector4f(mySpecularColor.r, mySpecularColor.g, mySpecularColor.b, mySpecularColor.a));
    }
}

void
MaterialExporter::exportPhongEFeatures(const MFnMesh * theMesh, const MObject & theShaderNode,
                                        y60::MaterialBuilder & theBuilder,
                                        y60::SceneBuilder & theSceneBuilder)
{
    exportReflectiveFeatures(theMesh, theShaderNode, theBuilder, theSceneBuilder);
    MStatus myStatus;
    float myHightlightSize = MFnPhongEShader(theShaderNode).highlightSize(& myStatus);
    
    float myShininess = asl::maximum(0.0f, 128.0f - (myHightlightSize * 100.0f)); // experimental
    setPropertyValue<float>(theBuilder.getNode(), "float", y60::SHININESS_PROPERTY, myShininess);
}

void
MaterialExporter::exportPhongFeatures(const MFnMesh * theMesh, const MObject & theShaderNode,
                                        y60::MaterialBuilder & theBuilder,
                                        y60::SceneBuilder & theSceneBuilder)
{
    exportReflectiveFeatures(theMesh, theShaderNode, theBuilder, theSceneBuilder);
    MStatus myStatus;
    float myShininess = MFnPhongShader(theShaderNode).cosPower(& myStatus);
    if (myStatus == MStatus::kFailure) {
        throw ExportException("Could not get shininess from node",
                              "MaterialExporter::exportPhongFeatures");
    }

    // Convert to 0 - 128 range of OpenGL
    myShininess = (myShininess / 100) * 128;

    setPropertyValue<float>(theBuilder.getNode(), "float", y60::SHININESS_PROPERTY, myShininess);
}

void
MaterialExporter::exportBlinnFeatures(const MFnMesh * theMesh, const MObject & theShaderNode,
                                        y60::MaterialBuilder & theBuilder,
                                        y60::SceneBuilder & theSceneBuilder)
{
    exportReflectiveFeatures(theMesh, theShaderNode, theBuilder, theSceneBuilder);
    MStatus myStatus;
    float myEccentricity = MFnBlinnShader(theShaderNode).eccentricity(& myStatus);
    if (myStatus == MStatus::kFailure) {
        throw ExportException("Could not get shininess from node",
                              "MaterialExporter::exportBlinnFeatures");
    }

    // Convert to 0 - 128 range of OpenGL
    float myShininess = (1.0f-myEccentricity) * 128;

    setPropertyValue<float>(theBuilder.getNode(), "float", y60::SHININESS_PROPERTY, myShininess);
}

std::string
MaterialExporter::createMaterial(const MFnMesh * theMesh, const MObject & theShaderNode, 
                                 y60::SceneBuilder & theSceneBuilder, const SpecialFeatures & theSpecialFeatures)
{
    std::string myMaterialName(MFnDependencyNode(theShaderNode).name().asChar());
    y60::MaterialBuilder myMaterialBuilder(myMaterialName, _myInlineTexturesFlag);
    std::string myMaterialId = theSceneBuilder.appendMaterial(myMaterialBuilder);

    DB(AC_TRACE << "MaterialExporter::createMaterial() - Exporting material: " << theShaderNode.apiTypeStr()
           << " with shader: " << myMaterialName);

    // Check for custom attributes
    MString myPhysics = "";
    if (getCustomAttribute(theShaderNode, "ac_physics", myPhysics)) {
        cerr << "Found physics in " << myMaterialName << " with value: " << myPhysics.asChar() << endl;
        myMaterialBuilder.addFeature("physics", VectorOfRankedFeature(100, myPhysics.asChar()));
    }

    // Add special features collected in shape exporter
    for (unsigned i = 0; i < theSpecialFeatures.size(); ++i) {
        myMaterialBuilder.addFeature(theSpecialFeatures[i].classname, 
               VectorOfRankedFeature(100, theSpecialFeatures[i].values) );
    }

    VectorOfRankedFeature myLightingFeature;
    exportShader(theMesh, theShaderNode, myMaterialBuilder, theSceneBuilder, myLightingFeature);

    myMaterialBuilder.setType(myLightingFeature);
    myMaterialBuilder.setTransparencyFlag(checkTransparency(theShaderNode));
    _myMaterialNameMap[myMaterialName] = myMaterialId;

    return myMaterialId;
}

void
MaterialExporter::exportShader(const MFnMesh * theMesh, const MObject & theShaderNode,
                               y60::MaterialBuilder & theMaterialBuilder,
                               y60::SceneBuilder & theSceneBuilder,
                               VectorOfRankedFeature & theLightingFeature)
{
    switch (theShaderNode.apiType()) {
        case MFn::kLambert: {
            createLightingFeature(theLightingFeature, y60::LAMBERT);
            exportLambertFeatures(theMesh, theShaderNode, theMaterialBuilder, theSceneBuilder);
            break;
        }
        case MFn::kSurfaceShader: {
            createLightingFeature(theLightingFeature, y60::UNLIT);
            exportUnlitFeatures(theMesh, theShaderNode, theMaterialBuilder, theSceneBuilder);
            break;
        }
        case MFn::kPhongExplorer: {
            createLightingFeature(theLightingFeature, y60::PHONG);
            exportLambertFeatures(theMesh, theShaderNode, theMaterialBuilder, theSceneBuilder);
            exportPhongEFeatures(theMesh, theShaderNode, theMaterialBuilder, theSceneBuilder);
            break;
        }
        case MFn::kPhong: {
            createLightingFeature(theLightingFeature, y60::PHONG);
            exportLambertFeatures(theMesh, theShaderNode, theMaterialBuilder, theSceneBuilder);
            exportPhongFeatures(theMesh, theShaderNode, theMaterialBuilder, theSceneBuilder);
            break;
        }
        case MFn::kBlinn: {
            createLightingFeature(theLightingFeature, y60::PHONG);
            exportLambertFeatures(theMesh, theShaderNode, theMaterialBuilder, theSceneBuilder);
            exportBlinnFeatures(theMesh, theShaderNode, theMaterialBuilder, theSceneBuilder);
            break;
        }

        case MFn::kLayeredShader: {
            // dumpAttributes(theShaderNode);
            MPlug myInputPlug = getPlug(theShaderNode,"inputs");
            if (!myInputPlug.isArray()) {
                throw ExportException(string("kLayeredShader input plug '")+
                            myInputPlug.name().asChar()+"' is not an array. aborting.",
                            "MaterialExporter::exportShader()");
            }
            for (unsigned i = 0; i < myInputPlug.numElements(); ++i) {
                // Find connected shader node
                MPlug myColorPlug;
                MObject childShaderNode;
                getChildPlugByName("color", myInputPlug[i], theShaderNode, myColorPlug);
                getConnectedNode(myColorPlug, childShaderNode);

                // recurse
                exportShader(theMesh, childShaderNode, theMaterialBuilder, theSceneBuilder, theLightingFeature);
            }
            break;
        }

        default:
            throw ExportException(std::string("Unsupported shader: ") + theShaderNode.apiTypeStr(),
                                  "MaterialExporter::exportShader()");
    }
}


MObject
MaterialExporter::getMaterial(MIntArray & theIndices, MObjectArray & theShadingGroups,
                              unsigned theFaceIndex)
{
    MAKE_SCOPE_TIMER(MaterialExporter_getMaterial);

    int myMaterialIndex  = theIndices[theFaceIndex];

    if (myMaterialIndex != -1) {
        MAKE_SCOPE_TIMER(MaterialExporter_findShader);
        return findShader(theShadingGroups[myMaterialIndex]);
    }

    throw ExportException("Could not get material name", "MaterialExporter::getMaterial()");
}

std::string
MaterialExporter::createFaceMaterial(const MFnMesh * theMesh, const MObject & theMaterialNode, 
                                     y60::SceneBuilder & theSceneBuilder, const SpecialFeatures & theSpecialFeatures)
{
    std::string myMaterialName = std::string(MFnDependencyNode(theMaterialNode).name().asChar());
    StringMap::iterator it = _myMaterialNameMap.find(myMaterialName);
    if (it != _myMaterialNameMap.end()) {
        // If the shader has already been created, return its id
        return it->second;
    } else {
        // If it does not exist yet, we have to create it.
        return createMaterial(theMesh, theMaterialNode, theSceneBuilder, theSpecialFeatures);
    }
}

const VectorOfString &
MaterialExporter::getTextureNames(const std::string & theMaterialName) {
    StringVectorMap::iterator myTextureNames = _myMaterialUVMappedTexturesMap.find(theMaterialName);
    if (myTextureNames != _myMaterialUVMappedTexturesMap.end()) {
        return myTextureNames->second;
    }

    throw ExportException(std::string("Could not find textures for '") + theMaterialName + "'",
            "MaterialExporter::getTextureNames");
}

bool
MaterialExporter::hasUVMappedTextures(const std::string & theMaterialName) {
    return (_myMaterialUVMappedTexturesMap.find(theMaterialName) != _myMaterialUVMappedTexturesMap.end());
}

void
MaterialExporter::setBaseDirectory(const std::string & theDirectory) {
    _myBaseDirectory = theDirectory;
}

void
MaterialExporter::stripBaseDir(std::string & theFileName) const {
#ifdef WIN32
    // We need to convert basedir and filename toupper, before comparing, because under windows
    // there are differences :( [ch]
    std::string myUpperFileName(theFileName);
    std::transform(theFileName.begin(), theFileName.end(),
                   myUpperFileName.begin(), std::toupper);

    std::string myUpperBaseName(_myBaseDirectory);
    std::transform(_myBaseDirectory.begin(), _myBaseDirectory.end(),
                   myUpperBaseName.begin(), std::toupper);

    if (myUpperFileName.find(myUpperBaseName) == 0) {
        theFileName = theFileName.substr(_myBaseDirectory.length());
    }
#else
    if (theFileName.find(_myBaseDirectory) == 0) {
        theFileName = theFileName.substr(_myBaseDirectory.length());
    }
#endif
}

std::string
MaterialExporter::getStrippedTextureFilename(const MPlug & theTexturePlug) const {
    MObject theFileNode;
    getConnectedNode(theTexturePlug, theFileNode);
    MString myFileName("");
    MPlug myFilenamePlug = MFnDependencyNode(theFileNode).findPlug("fileTextureName");

    myFilenamePlug.getValue(myFileName);

    std::string myFilename(myFileName.asChar());
    stripBaseDir(myFilename);
    return myFilename;
}

bool
MaterialExporter::checkAlphaTexture(const MObject & theShaderNode, const std::string thePlugName) {
    MStatus myStatus;

    MPlug myPlug = getPlug(theShaderNode, thePlugName.c_str());

    MPlugArray myPlugArray;
    myPlug.connectedTo(myPlugArray, true, false, & myStatus);
    if (myStatus == MS::kFailure) {
        throw ExportException("Could not get destination plug array",
                "MaterialExporter::checkAlphaTexture()");
    }

    if (myPlugArray.length() > 1) {
        throw ExportException("Unsupported type of color plug.",
                "MaterialExporter::checkAlphaTexture");
    }

    if (myPlugArray.length() > 0) {
        return true;
    }

    return false;
}

bool
MaterialExporter::checkTransparency(const MObject & theShaderNode) {
    bool myTransparencyFlag = false;
    DB(dumpAttributes(theShaderNode));

    MStatus myStatus;
    switch (theShaderNode.apiType()) {
        case MFn::kLambert:
        case MFn::kBlinn:
        case MFn::kPhongExplorer:
        case MFn::kPhong: {
            if (checkAlphaTexture(theShaderNode, "transparency")) {
                myTransparencyFlag = true;
            } else {
                MColor myTransparency = MFnLambertShader(theShaderNode).transparency(& myStatus);
                if (myTransparency.r != 0 || myTransparency.g != 0
                    || myTransparency.b != 0 || myTransparency.a < 1.0) {
                    myTransparencyFlag = true;
                }
            }
            break;
        }

        case MFn::kSurfaceShader: {
            if (checkAlphaTexture(theShaderNode, "outTransparency")) {
                myTransparencyFlag = true;
            } else {
                MPlug myTransparencyPlugR = MFnDependencyNode(theShaderNode).findPlug("outTransparencyR");
                MPlug myTransparencyPlugG = MFnDependencyNode(theShaderNode).findPlug("outTransparencyG");
                MPlug myTransparencyPlugB = MFnDependencyNode(theShaderNode).findPlug("outTransparencyB");
                double myColorR;
                double myColorB;
                double myColorG;
                myTransparencyPlugR.getValue(myColorR);
                myTransparencyPlugG.getValue(myColorG);
                myTransparencyPlugB.getValue(myColorB);
                if (myColorR != 0 || myColorG != 0 || myColorB != 0) {
                    myTransparencyFlag = true;
                }
            }
            break;
        }
        case MFn::kLayeredShader: {
            myTransparencyFlag = true; // TODO: recurse into shaders
            break;
        }
        default:
            throw ExportException(std::string("Unsupported shader: ") + theShaderNode.apiTypeStr(),
                                  PLUS_FILE_LINE);
    }

    return myTransparencyFlag;
}

/**
 * Transform alpha into color scale and color bias for layered textures
 * to omit the influence of a single components alpha on the entire texture
 *
 * @param theAlpha
 * @param theBlendMode
 * @param &theColorScale
 * @param &theColorBias
 * @return
 */
void
MaterialExporter::calcColorScaleAndBiasFromAlpha(const float theAlpha,
                                                 const MayaBlendMode theBlendMode,
                                                 asl::Vector4f &theColorScale,
                                                 asl::Vector4f &theColorBias)
{
    float myAlphaScale = 1.0f;
    float myAlphaBias  = 0.0f;
    float myScale      = 1.0f;
    float myBias       = 0.0f;

    switch (theBlendMode) {
        case MAYA_BLEND_MULTIPLY:
            myScale = theAlpha;
            myBias  = 1 - theAlpha;
            break;
        case MAYA_BLEND_ADD:
            myScale = theAlpha;
            break;
        case MAYA_BLEND_NONE:
            //nothing to be done
            break;
        case MAYA_BLEND_OVER:
            myAlphaScale = theAlpha;
            break;
        default:
            displayWarning(string("calcColorScaleAndBiasFromAlpha: not yet implemented for blendmode: ")
                           + getStringFromEnum(theBlendMode, MayaBlendModesString));
    }

    theColorScale = asl::Vector4f(myScale, myScale, myScale, myAlphaScale);
    theColorBias  = asl::Vector4f(myBias, myBias, myBias, myAlphaBias);
}
