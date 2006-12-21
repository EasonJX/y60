//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "MaterialExporter.h"

#include "Exceptions.h"
#include "CinemaHelpers.h"

#include <asl/Matrix4.h>
#include <y60/property_functions.h>
#include <y60/ShapeBuilder.h>
#include <y60/PropertyNames.h>
#include <y60/Image.h>

#if API_VERSION >= 8500
#include <Xbitmap.h>
#endif

#include <string>

using namespace std;
using namespace asl;
using namespace y60;

const float AMBIENT_COLOR = 0.4;

MaterialExporter::MaterialExporter(y60::SceneBuilderPtr theSceneBuilder,
                                   const Filename & theDocumentPath, bool theInlineTextures) :
    _mySceneBuilder(theSceneBuilder), _myDocumentPath(theDocumentPath), _myInlineTextures(theInlineTextures),
    _myMaterialBuilder(0), _myDefaultMaterialBuilder(0)
{
    Image::allowInlineFlag = _myInlineTextures;
}

void
MaterialExporter::getColor(BaseChannel * theColorChannel, Vector4f & theColor) {
    BaseContainer myColorContainer = theColorChannel->GetData();

    Vector myMaterialColor = myColorContainer.GetVector(BASECHANNEL_COLOR_EX);
    Real   myBrightness    = myColorContainer.GetReal(BASECHANNEL_BRIGHTNESS_EX);

    theColor[0] = myMaterialColor.x * myBrightness;
    theColor[1] = myMaterialColor.y * myBrightness;
    theColor[2] = myMaterialColor.z * myBrightness;
    theColor[3] = 1;
}

ExportedMaterialInfo
MaterialExporter::createDefaultMaterial() {
    if (_myDefaultMaterialInfo._myMaterialName == "") {
        _myDefaultMaterialBuilder = MaterialBuilderPtr (new y60::MaterialBuilder("default_material"));
        _myDefaultMaterialInfo._myMaterialId = _mySceneBuilder->appendMaterial(*_myDefaultMaterialBuilder);
        _myDefaultMaterialInfo._myMaterialName = "default_material";
        _myDefaultMaterialInfo._myTextureCount = 0;
        _myDefaultMaterialInfo._myTexureMapping.clear();

        VectorOfRankedFeature myLightingRequirements;
        createLightingFeature(myLightingRequirements, y60::LAMBERT);
           _myDefaultMaterialBuilder->setType(myLightingRequirements);

        setPropertyValue<asl::Vector4f>(_myDefaultMaterialBuilder->getNode(), "vector4f", y60::DIFFUSE_PROPERTY, Vector4f(0.8, 0.8, 0.8, 1));
        setPropertyValue<float>(_myDefaultMaterialBuilder->getNode(), "float", y60::SHININESS_PROPERTY, 64);
        setPropertyValue<asl::Vector4f>(_myDefaultMaterialBuilder->getNode(), "vector4f", y60::SPECULAR_PROPERTY, Vector4f(0, 0, 0, 1));
        setPropertyValue<asl::Vector4f>(_myDefaultMaterialBuilder->getNode(), "vector4f", y60::AMBIENT_PROPERTY, Vector4f(AMBIENT_COLOR, AMBIENT_COLOR, AMBIENT_COLOR, 1));
        _myDefaultMaterialBuilder->computeRequirements();
    }
    _myMaterialBuilder = _myDefaultMaterialBuilder;
    return _myDefaultMaterialInfo;
}

bool
MaterialExporter::exportTexture(Material* theMaterial, y60::MaterialBuilderPtr theMaterialBuilder,
                                y60::SceneBuilder & theSceneBuilder,
                                BaseContainer * theContainer,
                                const TextureUsage & theUsage, TextureTag * theTextureTag,
                                const asl::Vector3f & theMinCoord, const asl::Vector3f & theMaxCoord,
								bool isAlphaChannel, bool isEnvMap)
{
	String myMaterialName = theMaterial->GetName();
    if (!theContainer) {
        throw ExportException(std::string("Export texture with undefined cinema container"),
                                            "MaterialExporter::writeMaterial()");
    }

    bool myDiffuseColorFlag = false;
    TextureApplyMode myApplyMode = y60::DECAL;

    if (theUsage == y60::BUMP) {
        myApplyMode = y60::DECAL;
    } else {
        LONG myMixMode = theContainer->GetReal(BASECHANNEL_MIXMODE_EX);
        switch (myMixMode) {
            case MATERIAL_TEXTUREMIXING_NORMAL:
                myApplyMode = y60::MODULATE;
                break;
            case MATERIAL_TEXTUREMIXING_MULTIPLY:
                GePrint(myMaterialName + ": Texture mixmode MULTIPLY not implemented; using MODULATE");
                myApplyMode = y60::MODULATE;
                myDiffuseColorFlag = true;
                break;
            case MATERIAL_TEXTUREMIXING_ADD:
                myApplyMode = y60::ADD;
                break;
            case MATERIAL_TEXTUREMIXING_SUBTRACT:
                GePrint(myMaterialName + ": Texture mixmode SUBTRACT not implemented; using DECAL");
                break;
            default:
                throw ExportException(std::string("Unsupported texture mix mode: ") + asl::as_string(myMixMode),
                                                "MaterialExporter::writeMaterial()");
        }
    }

    // internal texture format
    std::string myInternalFormat = "";
	if (isAlphaChannel) {
		myInternalFormat = TEXTURE_INTERNAL_FORMAT_INTENSITY;
	}

    // color scale
    Real myMixStrength = theContainer->GetReal(BASECHANNEL_MIXSTRENGTH_EX, 1.0);
    GePrint("isAlphaChannel=" + LongToString(isAlphaChannel) + " mix=" + RealToString(myMixStrength));
    asl::Vector4f myColorScale;
    if (isAlphaChannel) {
        myColorScale = asl::Vector4f(myMixStrength, myMixStrength, myMixStrength, myMixStrength);
    } else {
        myColorScale = asl::Vector4f(myMixStrength, myMixStrength, myMixStrength, 1);
    }

    // setup color bias so that colors aren't blacked out
    asl::Vector4f myColorBias = asl::Vector4f(1,1,1,1) - myColorScale;
    GePrint(myMaterialName + ": colorScale=" + String(asl::as_string(myColorScale).c_str()) + " colorBias=" + String(asl::as_string(myColorBias).c_str()));

    // Texture
    std::string myImageId = "";
    String myTextureName = theContainer->GetString(BASECHANNEL_TEXTURE);
    if (myTextureName.GetLength()) {
        std::string myTextureFilename = getTexturePath(_myDocumentPath, myTextureName);
        if (theMaterialBuilder->isMovie(myTextureFilename)) {
            LONG myTimeMode = theContainer->GetLong(BASECHANNEL_TIME_MODE);
            unsigned myLoopCount = 0;
#if API_VERSION >= 8500
            switch(myTimeMode) {
                case BITMAPSHADER_TIMING_MODE_SIMPLE:
                    myLoopCount = 1;
                    break;
                case BITMAPSHADER_TIMING_MODE_LOOP:
                    myLoopCount = 0;
                    break;
                case BITMAPSHADER_TIMING_MODE_PINGPONG:
                    myLoopCount = 0;
                    displayMessage(std::string("Ping-Pong Mode is not supported: ") + getString(myTextureName));
                    break;
            }
#endif
            myImageId = theMaterialBuilder->createMovie(theSceneBuilder, getString(myTextureName), myTextureFilename,
			                                            myLoopCount, myColorScale, myColorBias, myInternalFormat);
        } else {
            bool myCreateMipmapFlag = (theUsage == PAINT);

            // wrap mode
            TextureWrapMode myWrapMode = REPEAT; // TODO: extract wrap mode

            myImageId = theMaterialBuilder->createImage(theSceneBuilder,
                                                        getString(myTextureName), myTextureFilename,
                                                        theUsage, myCreateMipmapFlag,
                                                        myColorScale, myColorBias,
                                                        SINGLE, myWrapMode, myInternalFormat);
        }
    }

    // Texture projection
    std::string myTextureMappingMode = "";

    Matrix4f myTextureMatrix;
    myTextureMatrix.makeIdentity();

    if (isEnvMap) {
        myTextureMappingMode = TexCoordMappingStrings[SPHERICAL_PROJECTION];
    } else {
        GeData myData;
        theTextureTag->GetParameter(DescID(TEXTURETAG_PROJECTION), myData, 0);
        LONG myCinemaTexCoordMode = myData.GetLong();
        myTextureMappingMode = TexCoordMappingStrings[myCinemaTexCoordMode];
        if (myCinemaTexCoordMode > PERSPECTIVE_PROJECTION ||
            myCinemaTexCoordMode == SPATIAL_PROJECTION ||
            myCinemaTexCoordMode == SHRINKMAP_PROJECTION ||
            myCinemaTexCoordMode == PERSPECTIVE_PROJECTION) {
            displayMessage(std::string("Mapping not supported: ") + myTextureMappingMode +
                                        " for texture: " + getString(myTextureName));
        } else {
            GePrint("Texture mapping mode=" + String(myTextureMappingMode.c_str()));
        }

        if (myTextureMappingMode != TEXCOORD_UV_MAP) {
		    Matrix4f myTexGenMatrix;
		    myTexGenMatrix.makeIdentity();

            Matrix4f myNormalizeObjectCoordsMatrix;
            myNormalizeObjectCoordsMatrix.makeIdentity();

            // object normalization to [-0.5,0.5]
            asl::Vector3f myObjectSize = difference(theMaxCoord, theMinCoord);
            myObjectSize[0] = myObjectSize[0] == 0 ? 1 : myObjectSize[0];
            myObjectSize[1] = myObjectSize[1] == 0 ? 1 : myObjectSize[1];
            myObjectSize[2] = myObjectSize[2] == 0 ? 1 : myObjectSize[2];

            asl::Vector3f myObjScale;
            myObjScale[0] =  1.0 / myObjectSize[0];
            myObjScale[1] = -1.0 / myObjectSize[1]; //Cinema 4D left-handed coord system
            myObjScale[2] =  1.0 / myObjectSize[2];

            asl::Vector3f myObjOffset;
            myObjOffset[0] = -0.5;
            myObjOffset[1] = -0.5;
            myObjOffset[2] = -0.5;

            myNormalizeObjectCoordsMatrix.setScale(myObjScale);
            myNormalizeObjectCoordsMatrix.setTranslation(myObjOffset);

            // scaling
            Vector myScale = theTextureTag->GetScale();

            asl::Matrix4f myScaleMatrix;
            myScaleMatrix.makeIdentity();
            myScaleMatrix.setScale(Vector3f( 0.5*float(myObjectSize[0] / myScale.x),
                                            0.5*float(myObjectSize[1] / myScale.y),
                                            0.5*float(myObjectSize[2] / myScale.z) ));

            //translating
            Vector myPos = theTextureTag->GetPos();

            asl::Matrix4f myPosMatrix;
            myPosMatrix.makeIdentity();
            myPosMatrix.setTranslation(Vector3f(-float(myPos.x),
                                                -float(myPos.y),
                                                float(myPos.z) ));

            //rotating
            Vector myRotation = theTextureTag->GetRot();

            asl::Matrix4f myPrePivot, myPostPivot, myRotMatrix;

            myPrePivot.makeIdentity();
            myPrePivot.setTranslation(Vector3f(0.5,0.5,0.5));

            myPostPivot.makeIdentity();
            myPostPivot.setTranslation(Vector3f(-0.5,-0.5,-0.5));

            myRotMatrix.makeIdentity();

            myRotMatrix.postMultiply(myPrePivot);
            myRotMatrix.rotateY(-myRotation.x);
            myRotMatrix.rotateX(-myRotation.y);
            myRotMatrix.rotateZ(myRotation.z);
            myRotMatrix.postMultiply(myPostPivot);

            // 2. apply operations
            myTexGenMatrix.postMultiply(myPosMatrix);
            myTexGenMatrix.postMultiply(myRotMatrix);
            myTexGenMatrix.postMultiply(myScaleMatrix);

            // 1. normalize object dimensions to [0,1]
            myTexGenMatrix.postMultiply(myNormalizeObjectCoordsMatrix);

            VectorOfVector4f myTexGenParams;
            myTexGenParams.push_back(myTexGenMatrix.getColumn(0));
            myTexGenParams.push_back(myTexGenMatrix.getColumn(1));
            myTexGenParams.push_back(myTexGenMatrix.getColumn(2));
            myTexGenParams.push_back(myTexGenMatrix.getColumn(3));
            setPropertyValue<VectorOfVector4f>(theMaterialBuilder->getNode(),
                    "vectorofvector4f", "texgenparam0", myTexGenParams);
        }
    }


    theMaterialBuilder->createTextureNode(myImageId, myApplyMode, theUsage,
                                          myTextureMappingMode, myTextureMatrix, 100, false, 0.0);
    return myDiffuseColorFlag;
}

bool
MaterialExporter::exportShader(PluginShader * theShader,
                               y60::MaterialBuilderPtr theMaterialBuilder,
                               Material* theMaterial,
                               y60::SceneBuilder & theSceneBuilder,
                               BaseContainer * theColorContainer,
                               TextureTag * theTextureTag,
                               const asl::Vector3f & theMinCoord, const asl::Vector3f & theMaxCoord,
							   bool isAlphaChannel, bool isEnvMap)
{
    GePrint("exportShader: Material=" + theMaterial->GetName());

    // iterate over all shaders of the material
    // supported shaders are:
    //                       Layer (multitexture) Sorry, attributes are not available via api, dismissed
    //                       Bitmap(single texture)
    bool myDiffuseColorFlag = false;
    while (theShader != NULL) {
        LONG myShaderType = theShader->GetType();
        GePrint("     Shader=" + theShader->GetName());
        switch (myShaderType) {
            case Xlayer:
                {
#if 1
                    GePrint("Ignoring layered Shader '" + theShader->GetName() + "' not supported");
#else
                    GePrint("Material has a layered shader, name:" +
                            theShader->GetName() + " with id: " +
                            LongToString(theShader->GetType()) );
                    PluginShader * myDownShader = theShader->GetDown();
                    myDiffuseColorFlag |= exportShader(myDownShader, theMaterialBuilder,
                                                       theMaterial, theSceneBuilder,
                                                       0, theTextureTag,
                                                       theMinCoord, theMaxCoord,
                                                       isAlphaChannel, isEnvMap);
#endif
                }
                break;
            case Xbitmap:
                {
                    BaseContainer *  myColorContainer = theColorContainer ? theColorContainer : theShader->GetDataInstance();
                    myDiffuseColorFlag |= exportTexture(theMaterial, theMaterialBuilder,
                                                        theSceneBuilder, myColorContainer,
                                                        y60::PAINT, theTextureTag,
                                                        theMinCoord, theMaxCoord,
                                                        isAlphaChannel, isEnvMap);
                }
                break;
            case Xgradient:
            case Xfilter:
            case Xnoise:
			case Xfusion:
            case Xbase:
			case Xnormaldirection:
                GePrint("Ignoring Shader: " + theShader->GetName());
                break;
            default:
                throw ExportException(std::string("Unsupported shader: ") +
                                      getString(theShader->GetName()),
                                      "MaterialExporter::writeMaterial()");

        };
        theShader = 0;//theShader->GetNext(); // iteration over shader will give shader from other channels, which might be disabled, TODO
    };
    return myDiffuseColorFlag;
}

ExportedMaterialInfo
MaterialExporter::initiateExport(BaseObject * theNode, TextureList theTextureList,
                                 y60::SceneBuilder & theSceneBuilder) {
    ExportedMaterialInfo myExportedMaterialInfo;
    unsigned myCinemaTexturesCount = theTextureList.size();
    if (theNode == 0 ||  myCinemaTexturesCount == 0) {
        return createDefaultMaterial();
    }

    // concatenate material name and collect cinemax materials
    _myMaterials.clear();
    std::string myY60MaterialName("");
    for (int myTextureIndex = 0; myTextureIndex < theTextureList.size(); myTextureIndex++) {
        MaterialTextureTagCombo myMatTexTagCombo;
        myMatTexTagCombo._myTextureTag = theTextureList[myTextureIndex].first;
        myMatTexTagCombo._myMaterial = static_cast<Material *>(myMatTexTagCombo._myTextureTag->GetMaterial());
        if (!myMatTexTagCombo._myMaterial) {
            throw ExportException("Material not defined", "MaterialExporter::writeMaterial()");
        }
        _myMaterials.push_back(myMatTexTagCombo);
        if (myTextureIndex == 0) {
            myY60MaterialName = getString(myMatTexTagCombo._myMaterial->GetName());
        } else {
            myY60MaterialName = myY60MaterialName + "_" + getString(myMatTexTagCombo._myMaterial->GetName());
        }
    }

    MaterialInfoMap::iterator myMaterialIt = _myMaterialMap.find(myY60MaterialName);
    if (myMaterialIt != _myMaterialMap.end()) {
        return myMaterialIt->second;
    }

    _myMaterialBuilder = y60::MaterialBuilderPtr(new y60::MaterialBuilder(myY60MaterialName, _myInlineTextures));
    myExportedMaterialInfo._myMaterialId = _mySceneBuilder->appendMaterial(*_myMaterialBuilder);
    myExportedMaterialInfo._myMaterialName = myY60MaterialName;

    // count y60 textures that need uvmaps
    for (int myMaterialIndex = 0; myMaterialIndex < _myMaterials.size(); myMaterialIndex++) {
        Material * myMaterial = _myMaterials[myMaterialIndex]._myMaterial;
        TextureTag * myTextureTag = _myMaterials[myMaterialIndex]._myTextureTag;
          if (myMaterial->GetChannelState(CHANNEL_COLOR)) {
              BaseChannel * myColorChannel = myMaterial->GetChannel(CHANNEL_COLOR);
              if (myColorChannel) {
                PluginShader  * myShader = myColorChannel->GetShader();
                if (myShader) {
                    LONG myShaderType = myShader->GetType();
                    if ( myShaderType == Xbitmap) {
                        GeData myData;
                        myTextureTag->GetParameter(DescID(TEXTURETAG_PROJECTION), myData, 0);
                        LONG myCinemaTexCoordMode = myData.GetLong();
                        myExportedMaterialInfo._myTexureMapping.push_back(myCinemaTexCoordMode);
                        myExportedMaterialInfo._myTextureCount++;
                    }
                }
            }
		  }
          if (myMaterial->GetChannelState(CHANNEL_ALPHA)) {
            BaseChannel * myAlphaChannel = myMaterial->GetChannel(CHANNEL_ALPHA);
            if (myAlphaChannel) {
				PluginShader  * myShader = myAlphaChannel->GetShader();
				if (myShader) {
					LONG myShaderType = myShader->GetType();
					if ( myShaderType == Xbitmap) {
						GeData myData;
						myTextureTag->GetParameter(DescID(TEXTURETAG_PROJECTION), myData, 0);
						LONG myCinemaTexCoordMode = myData.GetLong();
						myExportedMaterialInfo._myTexureMapping.push_back(myCinemaTexCoordMode);
						myExportedMaterialInfo._myTextureCount++;
					}
				}
			}
          }
    }
    return myExportedMaterialInfo;
}

void
MaterialExporter::writeMaterial(const ExportedMaterialInfo & theMaterialInfo, BaseObject * theNode,
                                TextureList theTextureList, y60::SceneBuilder & theSceneBuilder,
                                const asl::Vector3f & theMinCoord, const asl::Vector3f & theMaxCoord)
{
    if (theNode == 0) {
        return;
    }

    if (!theTextureList.empty()) {
        // Get connected material
        //ExportedMaterialInfo myExportedMaterialInfo;
        //myExportedMaterialInfo._myTextureCount = 0;

        // Check if material has alread been exported
        if (_myMaterialMap.find(theMaterialInfo._myMaterialName) != _myMaterialMap.end()) {
            return;
        }

        VectorOfRankedFeature myLightingRequirements;
        bool myMaterialAlphaFlag = false;

        for (unsigned myMaterialIndex = 0; myMaterialIndex < _myMaterials.size(); ++myMaterialIndex) {
            Material * myMaterial = _myMaterials[myMaterialIndex]._myMaterial;
            TextureTag * myTextureTag = _myMaterials[myMaterialIndex]._myTextureTag;

            // Export diffuse color and texture
            y60::LightingModel myLightingType = y60::LAMBERT;
            if (myMaterial->GetChannelState(CHANNEL_COLOR)) {
                BaseChannel * myColorChannel = myMaterial->GetChannel(CHANNEL_COLOR);
                if (myColorChannel) {
                    // Color texture
                    BaseContainer myColorContainer = myColorChannel->GetData();
                    String myTextureName = myColorContainer.GetString(BASECHANNEL_TEXTURE);
                    String myTextureShaderid = myColorContainer.GetString(BASECHANNEL_SHADERID);
                    GePrint("### " + myMaterial->GetName() + ": Texture=" + myTextureName + " ShaderID=" + myTextureShaderid);

                    bool myDiffuseColorFlag = false;
                    PluginShader  * myShader = myColorChannel->GetShader();
                    if (!myShader) {
                        // Note: We do not export the diffuse color if a texture is present, because we are
                        // currently not able to support the mix-mode feature of cinema 4d
                        asl::Vector4f myDiffuseColor;
                        getColor(myColorChannel, myDiffuseColor);
                        setPropertyValue<asl::Vector4f>(_myMaterialBuilder->getNode(), "vector4f", y60::DIFFUSE_PROPERTY, myDiffuseColor);

                        asl::Vector4f myAmbientColor = product(myDiffuseColor, AMBIENT_COLOR);
                        myAmbientColor[3] = 1.0f;
                        setPropertyValue<asl::Vector4f>(_myMaterialBuilder->getNode(), "vector4f", y60::AMBIENT_PROPERTY, myAmbientColor);
                    } else {
                        myDiffuseColorFlag = exportShader(myShader, _myMaterialBuilder,
                                                          myMaterial,theSceneBuilder,
                                                          &myColorContainer, myTextureTag,
                                                          theMinCoord, theMaxCoord);

                        // Material base color
                        Vector4f myColor(1, 1, 1, 1);
                        if (myDiffuseColorFlag) {
                            getColor(myColorChannel, myColor);
                            setPropertyValue<asl::Vector4f>(_myMaterialBuilder->getNode(), "vector4f", y60::DIFFUSE_PROPERTY, myColor);
                        }

                        myColor *= AMBIENT_COLOR;
                        myColor[3] = 1.0;
                        setPropertyValue<asl::Vector4f>(_myMaterialBuilder->getNode(), "vector4f", y60::AMBIENT_PROPERTY, myColor);
                    }
                }
            }

            // Transparency / Alpha
            if (myMaterial->GetChannelState(CHANNEL_TRANSPARENCY)) {
                BaseChannel * myTransparencyChannel = myMaterial->GetChannel(CHANNEL_TRANSPARENCY);
                if (myTransparencyChannel) {
                    BaseContainer myColorContainer = myTransparencyChannel->GetData();
                    Real myMixStrength = myColorContainer.GetReal(BASECHANNEL_MIXSTRENGTH_EX, 1.0);
                    Real myBrightness = myColorContainer.GetReal(BASECHANNEL_BRIGHTNESS_EX);

                    Vector4f myDiffuseColor(1, 1, 1, 1);
                    getColor(myTransparencyChannel, myDiffuseColor); // we do not know what to do with this color value (vs)
                    if (hasPropertyValue<asl::Vector4f>(_myMaterialBuilder->getNode(), "vector4f", y60::DIFFUSE_PROPERTY) ) {
                        myDiffuseColor = getPropertyValue<asl::Vector4f>(_myMaterialBuilder->getNode(), "vector4f", y60::DIFFUSE_PROPERTY);
                    }
                    myDiffuseColor[3] = myBrightness;
                    setPropertyValue<asl::Vector4f>(_myMaterialBuilder->getNode(), "vector4f", y60::DIFFUSE_PROPERTY, myDiffuseColor);
                }
            }

            // Luminance
            if (myMaterial->GetChannelState(CHANNEL_LUMINANCE)) {
                BaseChannel * myLuminanceChannel = myMaterial->GetChannel(CHANNEL_LUMINANCE);
                if (myLuminanceChannel) {
                    asl::Vector4f myEmissiveColor;
                    BaseContainer myColorContainer = myLuminanceChannel->GetData();
                    // Brightness has already scaled luminance channel color
                    getColor(myLuminanceChannel, myEmissiveColor);

                    Real myBrightness = myColorContainer.GetReal(BASECHANNEL_BRIGHTNESS_EX);
                    GePrint("luminance brightness=" + RealToString(myBrightness));
                    //myEmissiveColor[0] *= myBrightness;
                    //myEmissiveColor[1] *= myBrightness;
                    //myEmissiveColor[2] *= myBrightness;
                    setPropertyValue<asl::Vector4f>(_myMaterialBuilder->getNode(), "vector4f", y60::EMISSIVE_PROPERTY, myEmissiveColor);
                }
            }

            // Alpha
            if (myMaterial->GetChannelState(CHANNEL_ALPHA)) {
                BaseChannel * myAlphaChannel = myMaterial->GetChannel(CHANNEL_ALPHA);
                if (myAlphaChannel) {
			        BaseContainer myColorContainer = myAlphaChannel->GetData();
			        String myTextureName = myColorContainer.GetString(BASECHANNEL_TEXTURE);
			        String myTextureShaderid = myColorContainer.GetString(BASECHANNEL_SHADERID);
                    GePrint("### " + myMaterial->GetName() + ": Alpha texture=" + myTextureName);
	                PluginShader  * myShader = myAlphaChannel->GetShader();
                    exportShader(myShader, _myMaterialBuilder, myMaterial,theSceneBuilder,
                                 &myColorContainer, myTextureTag, theMinCoord, theMaxCoord, true);
		        }
		    }

		    myMaterialAlphaFlag = myMaterialAlphaFlag | (myMaterial->GetChannelState(CHANNEL_ALPHA)==TRUE) |
                                                        (myMaterial->GetChannelState(CHANNEL_TRANSPARENCY)==TRUE) ;

            // Specular color
            Vector4f mySpecularColor(0, 0, 0, 1);
            bool foundSpecularColor = false;
            if (myMaterial->GetChannelState(CHANNEL_SPECULARCOLOR)) {
                BaseChannel * mySpecularColorChannel = myMaterial->GetChannel(CHANNEL_SPECULARCOLOR);
                if (mySpecularColorChannel) {
                    myLightingType = y60::PHONG;
                    getColor(mySpecularColorChannel, mySpecularColor);
                    foundSpecularColor = true;
                }
            }

            // Specular material
            if (myMaterial->GetChannelState(CHANNEL_SPECULAR)) {
                BaseChannel * mySpecularChannel = myMaterial->GetChannel(CHANNEL_SPECULAR);
                if (mySpecularChannel) {
                    BaseContainer * mySpecularContainer = myMaterial->GetDataInstance();
                    LONG mySpecularMode = mySpecularContainer->GetLong(MATERIAL_SPECULAR_MODE);
                    switch (mySpecularMode) {
                        case MATERIAL_SPECULAR_MODE_PLASTIC:
                        case MATERIAL_SPECULAR_MODE_METAL:
                        case MATERIAL_SPECULAR_MODE_COLORED:
                            if (!foundSpecularColor) {
                                mySpecularColor = Vector4f(1, 1, 1, 1);
                            }
                            break;
                    }

                    // Shininess
                    myLightingType = y60::PHONG;
                    float mySpecularWidth      = mySpecularContainer->GetReal(MATERIAL_SPECULAR_WIDTH);
                    float mySpecularHeight     = mySpecularContainer->GetReal(MATERIAL_SPECULAR_HEIGHT);
                    float mySpecularFallOff    = mySpecularContainer->GetReal(MATERIAL_SPECULAR_FALLOFF);
                    float mySpecularInnerWidth = mySpecularContainer->GetReal(MATERIAL_SPECULAR_INNERWIDTH);

                    // TODO: Find the best formula (use vrml exporter in cinema for reference results)
                    // Suggested formula: shininess = (specular_width + (1 - specular_height) * 128) / 2
                    float myShininess = (mySpecularWidth + (1 - mySpecularHeight) * 128) / 2;
                    setPropertyValue<float>(_myMaterialBuilder->getNode(), "float", y60::SHININESS_PROPERTY, myShininess);
                    setPropertyValue<asl::Vector4f>(_myMaterialBuilder->getNode(), "vector4f", y60::SPECULAR_PROPERTY, mySpecularColor);
                }
            }

            // Bumpmap
            if (myMaterial->GetChannelState(CHANNEL_BUMP)) {
                BaseChannel * myChannel = myMaterial->GetChannel(CHANNEL_BUMP);
                if (myChannel) {
                    BaseContainer myContainer = myChannel->GetData();
                    String myBumpMapName = myContainer.GetString(BASECHANNEL_TEXTURE);
                    if (myBumpMapName.GetLength()) {
#if 1
                        GePrint("### " + myMaterial->GetName() + ": Bumpmap is not supported: " + myBumpMapName);
#else
                        exportTexture(myMaterial, _myMaterialBuilder, theSceneBuilder, &myContainer, y60::BUMP, myTextureTag, theMinCoord, theMaxCoord, false);
                        _myMaterialBuilder->needTextureFallback(true);
#endif
                    }
                }
            }

            // Environment map
            if (myMaterial->GetChannelState(CHANNEL_ENVIRONMENT)) {
                BaseChannel * myChannel = myMaterial->GetChannel(CHANNEL_ENVIRONMENT);
                if (myChannel) {
                    BaseContainer myContainer = myChannel->GetData();
                    String myTextureName = myContainer.GetString(BASECHANNEL_TEXTURE);
                    GePrint("### " + myMaterial->GetName() + ": Environment texture=" + myTextureName);
                    if (myTextureName.GetLength() ) {
                        exportTexture(myMaterial,
                                      _myMaterialBuilder, theSceneBuilder,
                                      &myContainer, y60::PAINT,
                                      myTextureTag, theMinCoord, theMaxCoord, false, true);
                    }
                }
            }

            // build material requirements
            createLightingFeature(myLightingRequirements, myLightingType);
                _myMaterialBuilder->setType(myLightingRequirements);
        }

        _myMaterialBuilder->setTransparencyFlag(myMaterialAlphaFlag);
        //myExportedMaterialInfo._myTextureCount = _myMaterialBuilder->getTextureCount();
        _myMaterialMap[theMaterialInfo._myMaterialName] = theMaterialInfo;
        _myMaterialBuilder->computeRequirements();

        GePrint("+++ Exported material '" + String(theMaterialInfo._myMaterialName.c_str()) + "'");
        //return myExportedMaterialInfo;
        //return _myMaterialBuilder;
    } else {
        writeMaterial(theMaterialInfo, theNode->GetUp(), theSceneBuilder,theMinCoord, theMaxCoord);
    }
}

void
MaterialExporter::writeMaterial(const ExportedMaterialInfo & theMaterialInfo, BaseObject * theNode,
                                y60::SceneBuilder & theSceneBuilder,
                                const asl::Vector3f & theMinCoord, const asl::Vector3f & theMaxCoord)
{
    TextureTag * myTextureTag = 0;
    TextureList myTextureList;
    if (theNode) {
        for (LONG ti = 0; (myTextureTag = static_cast<TextureTag*>(theNode->GetTag(Ttexture, ti))); ++ti) {
            UVWTag * myUvTag = static_cast<UVWTag*>(theNode->GetTag(Tuvw, ti));
            myTextureList.push_back(std::pair<TextureTag*,UVWTag*>(myTextureTag, myUvTag));
        }
        GePrint("*** Texture=" + theNode->GetName() + " count=" + LongToString(myTextureList.size()));
    }

    writeMaterial(theMaterialInfo, theNode, myTextureList, theSceneBuilder, theMinCoord, theMaxCoord);
}

const std::string
MaterialExporter::getTexturePath(Filename & theDocumentPath, String & theTextureFilename) {
    String mySearchPath;
    Filename myAbsoluteTexFileName;
    Filename myDummySuggestedFolder;

    std::string myDocumentPath = getString(theDocumentPath.GetString());

    if (GenerateTexturePath(theDocumentPath, theTextureFilename, myDummySuggestedFolder, &myAbsoluteTexFileName)) {
        LONG myPos;
        String myAbsoluteTexFileString = myAbsoluteTexFileName.GetString();
        if (myAbsoluteTexFileString.FindFirst(theDocumentPath.GetString(), &myPos, 0) ) {
            mySearchPath = myAbsoluteTexFileString.SubStr(theDocumentPath.GetString().GetLength() + 1, myAbsoluteTexFileString.GetLength());
        } else {
            mySearchPath = myAbsoluteTexFileString;
        }
    } else {
        throw ExportException(std::string("Could not find texture: ")
                              + getString(theTextureFilename),
                              "MaterialExporter::getTexturePath()");
    }

    return getString(mySearchPath);
}
