//============================================================================
// Copyright (C) 2000-2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#ifndef _ac_MaterialBuilder_h_
#define _ac_MaterialBuilder_h_

#include "BuilderBase.h"
#include <y60/RankedFeature.h>
#include <y60/NodeValueNames.h>
#include <y60/typedefs.h>

#include <dom/typedefs.h>
#include <asl/Ptr.h>
#include <asl/Matrix4.h>

#include <vector>

namespace y60 {
    class SceneBuilder;

    class MaterialBuilder : public BuilderBase {
    public:
        DEFINE_EXCEPTION(MaterialBuilderException, asl::Exception);

        MaterialBuilder(const std::string & theName, bool theInlineTextureFlag = true);
        virtual ~MaterialBuilder();

        void needTextureFallback(bool needTextureFallback);
        void setType(const VectorOfRankedFeature & theType);

        unsigned getTextureCount();
        bool isBumpMap(unsigned theTextureIndex) const;
        bool isMovie(const std::string & theFileName) const;

        void addFeature(const std::string & theClass, const VectorOfRankedFeature & theValue);

        dom::NodePtr createTextureUnitNode(const std::string & theTextureId,
                const TextureApplyMode & theApplyMode,
                const TextureUsage & theUsage,
                const std::string & theMappingMode,
                const asl::Matrix4f & theMatrix,
                bool theSpriteFlag = false, float theRanking = 100.0f,
                bool  isFallback = false, float theFallbackRanking = 0.0f);

        dom::NodePtr createTextureNode(SceneBuilder & theSceneBuilder,
                const std::string & theName, const std::string & theImageId,
                const TextureWrapMode & theWrapMode,
                bool  theCreateMipmapsFlag,
                const asl::Matrix4f & theMatrix,
                const std::string & theInternalFormat = "",
                const asl::Vector4f & theColorScale = asl::Vector4f(1,1,1,1),
                const asl::Vector4f & theColorBias = asl::Vector4f(0,0,0,0),
                bool allowSharing = true);

        dom::NodePtr createMovieNode(SceneBuilder & theSceneBuilder,
                const std::string & theName, const std::string & theFileName,
                unsigned theLoopCount);

        dom::NodePtr createImageNode(SceneBuilder & theSceneBuilder,
                const std::string & theName, const std::string & theFileName,
                const TextureUsage & theUsage,
                ImageType theType = SINGLE,
                const std::string & theResizeMode = IMAGE_RESIZE_SCALE,
                unsigned theDepth = 1,
                bool allowSharing = true);

/*
        void appendTexture(SceneBuilder & theSceneBuilder,
                           const std::string & theName,
                           const std::string & theFileName,
                           const std::string & theApplyMode,
                           const std::string & theUsage,
                           const std::string & theWrapMode,
                           const asl::Matrix4f & theMatrix,
                           float theRanking,
                           bool  isFallback,
                           float theFallbackRanking,
                           bool  theCreateMipmapsFlag,
                           float theAlpha = - 1.0,
                           bool  theSpriteFlag = false,
                           unsigned theDepth = 1);*/

        void appendCubemap(SceneBuilder & theSceneBuilder,
                           const std::string & theName,
                           const TextureUsage & theUsage,
                           const std::string & theFrontFileName,
                           const std::string & theRightFileName,
                           const std::string & theBackFileName,
                           const std::string & theLeftFileName,
                           const std::string & theTopFileName,
                           const std::string & theBottomFileName,
                           const TextureApplyMode & theApplyMode);

        void setTextureUVRepeat(const std::string & theTextureName, double theRepeatU, double theRepeatV);

        const std::string & getName() const;
        void computeRequirements();

        void setTransparencyFlag(bool theFlag);
        bool getTransparencyFlag() const;

    private:
        void checkState();
        void setup();
        bool _myRequirementsAdded;
        bool _needTextureFallback;
        bool _myInlineTextureFlag;

        // Requirements
        VectorOfRankedFeature _myMappingRequirements;
        VectorOfRankedFeature _myTextureRequirements;
        VectorOfRankedFeature _myLightingRequirements;

        static unsigned _myNextId;
    };

    typedef asl::Ptr<MaterialBuilder>  MaterialBuilderPtr;

    void createLightingFeature(VectorOfRankedFeature & theRequirement,
                               LightingModel  theLightingModelType);
}

#endif
