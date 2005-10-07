//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: MaterialBase.h,v $
//   $Author: christian $
//   $Revision: 1.32 $
//   $Date: 2005/03/24 23:36:03 $
//
//   Description:
//
//=============================================================================

#ifndef _ac_scene_MaterialBase_h_
#define _ac_scene_MaterialBase_h_

#include "Texture.h"
#include "MaterialParameter.h"
#include "PropertyListFacade.h"
#include "IShader.h"

#include <y60/CommonTags.h>
#include <y60/PropertyNames.h>
#include <dom/Facade.h>
#include <dom/PropertyPlug.h>
#include <dom/AttributePlug.h>
#include <dom/ChildNodePlug.h>

#include <asl/Exception.h>
#include <y60/DataTypes.h>
#include <dom/Nodes.h>
#include <string>
#include <vector>

namespace y60 {
    class TextureManager;
   
    DEFINE_EXCEPTION(MaterialBaseException, asl::Exception);
	static y60::VectorOfString getDefaultBlendFunction() {
		y60::VectorOfString myResult;
		myResult.push_back("src_alpha");
		myResult.push_back("one_minus_src_alpha");
		return myResult;
	}

#define DEFINE_MATERIAL_PROPERTY_TAG(theTagName, theType, thePropertyName, theDefault) \
	DEFINE_PROPERTY_TAG(theTagName,  MaterialPropertiesFacade, theType, y60::getTypeName<theType>(), thePropertyName,  PROPERTY_LIST_NAME, NAME_ATTRIB, theDefault);

    DEFINE_MATERIAL_PROPERTY_TAG(MaterialAmbientTag, asl::Vector4f, AMBIENT_PROPERTY, asl::Vector4f(0.0f,0.0f,0.0f,1));
    DEFINE_MATERIAL_PROPERTY_TAG(MaterialDiffuseTag, asl::Vector4f, DIFFUSE_PROPERTY, asl::Vector4f(1.0f,1.0f,1.0f,1));
    DEFINE_MATERIAL_PROPERTY_TAG(MaterialSpecularTag, asl::Vector4f, SPECULAR_PROPERTY, asl::Vector4f(0,0,0,1));
    DEFINE_MATERIAL_PROPERTY_TAG(MaterialEmissiveTag, asl::Vector4f, EMISSIVE_PROPERTY, asl::Vector4f(0,0,0,1));
	DEFINE_MATERIAL_PROPERTY_TAG(SurfaceColorTag, asl::Vector4f, SURFACE_COLOR_PROPERTY, asl::Vector4f(0.0f,0.0f,0.0f,1));
    DEFINE_MATERIAL_PROPERTY_TAG(ShininessTag, float, SHININESS_PROPERTY, float(1.0));
	DEFINE_PROPERTY_TAG(BlendFunctionTag, MaterialPropertiesFacade, y60::VectorOfString, SOM_VECTOR_STRING_NAME,
		                BLENDFUNCTION_PROPERTY, PROPERTY_LIST_NAME, NAME_ATTRIB,  getDefaultBlendFunction());

    DEFINE_PROPERTY_TAG(ReqLightingTag, MaterialRequirementFacade, y60::VectorOfRankedFeature, FEATURE_NODE_NAME,
		                LIGHTING_FEATURE, REQUIRES_LIST_NAME, NAME_ATTRIB,  y60::VectorOfRankedFeature(100,"unlit"));

	class MaterialPropertiesFacade :
		public PropertyListFacade,
		public MaterialAmbientTag::Plug,
		public MaterialDiffuseTag::Plug,
		public MaterialSpecularTag::Plug,
		public MaterialEmissiveTag::Plug,
		public SurfaceColorTag::Plug,
		public ShininessTag::Plug,
		public BlendFunctionTag::Plug
	{
		public:
			MaterialPropertiesFacade(dom::Node & theNode) :
				PropertyListFacade(theNode),
				MaterialAmbientTag::Plug(this),
				MaterialDiffuseTag::Plug(this),
				MaterialSpecularTag::Plug(this),
				SurfaceColorTag::Plug(this),
				ShininessTag::Plug(this),
				MaterialEmissiveTag::Plug(this),
				BlendFunctionTag::Plug(this)
			{}
			IMPLEMENT_FACADE(MaterialPropertiesFacade);
	};
	typedef asl::Ptr<MaterialPropertiesFacade, dom::ThreadingModel> MaterialPropertiesFacadePtr;

	class MaterialRequirementFacade :
		public PropertyListFacade,
		public ReqLightingTag::Plug
	{
		public:
			MaterialRequirementFacade(dom::Node & theNode) :
 				PropertyListFacade(theNode),
				ReqLightingTag::Plug(this)
			{}
			IMPLEMENT_FACADE(MaterialRequirementFacade);
	};
	typedef asl::Ptr<MaterialRequirementFacade, dom::ThreadingModel> MaterialRequirementFacadePtr;

	DEFINE_CHILDNODE_TAG(MaterialPropertiesTag, MaterialBase, MaterialPropertiesFacade, PROPERTY_LIST_NAME);
	DEFINE_CHILDNODE_TAG(MaterialRequirementTag, MaterialBase, MaterialRequirementFacade, REQUIRES_LIST_NAME);

	DEFINE_ATTRIBUT_TAG(TransparencyTag,   bool, TRANSPARENCY_ATTRIB, false);

    class MaterialBase :
		public dom::Facade,
        public IdTag::Plug,
		public MaterialPropertiesTag::Plug,
		public MaterialRequirementTag::Plug,
        public NameTag::Plug,
        public TransparencyTag::Plug
     {
        public:
            typedef std::vector<TexCoordMode> TexGenMode;
            typedef std::vector<TexGenMode>   TexGenModeList;

            typedef std::vector<asl::Vector4f> TexGenParams;
            typedef std::vector<TexGenParams>  TexGenParamsList;

            MaterialBase(dom::Node & theNode);
            IMPLEMENT_CHILD_FACADE(MaterialBase);

            virtual ~MaterialBase();

            MaterialBase(dom::Node & theNode, dom::Node & theDefaults);

            virtual unsigned getTextureCount() const;
            virtual const Texture & getTexture(unsigned myIndex) const;

            virtual void load(TextureManager & theTextureMananger);

            virtual void setup(dom::NodePtr theSceneNode) {}

            virtual void update(TextureManager & theTextureManager, const dom::NodePtr theImages);

            const MaterialParameterVectorPtr getVertexParameters() const;

            void setShader(IShaderPtr theShader);
            const IShaderPtr getShader() const { return _myShader; };
            IShaderPtr getShader() { return _myShader; };

            void mergeProperties(const dom::NodePtr & thePropertyNode);
            const LightingModel getLightingModel() const { return _myLightingModel; }

            TextureUsage MaterialBase::getTextureUsage(unsigned theTextureSlot) const;
            //bool hasTransparency() const;
            bool writesDepthBuffer() const;

            // texgen parameters
            void updateParams();

            bool hasTexGen() const {
                return _myTexGenFlag;
            }

            TexGenModeList getTexGenModes() const {
                return _myTexGenModes;
            }

			TexGenParamsList getTexGenParams() const {
                return _myTexGenParams;
            }

        protected:
            IShaderPtr                 _myShader;
        private:
            void addTextures(const dom::NodePtr theTextureListNode,
                              TextureManager & theTextureMananger);
            void addTexture(dom::NodePtr theTextureNode, TextureManager & theTextureManager);

            std::vector<TexturePtr> _myTextures;
            LightingModel           _myLightingModel;

            unsigned long     _myMaterialVersion;
            bool              _myTexGenFlag;
			TexGenModeList    _myTexGenModes;
			TexGenParamsList  _myTexGenParams;
     };


     typedef asl::Ptr<MaterialBase>       MaterialBasePtr;
     typedef std::vector<MaterialBasePtr> MaterialBasePtrVector;
	 typedef asl::Ptr<MaterialBase, dom::ThreadingModel> MaterialBaseFacadePtr;

}

#endif
