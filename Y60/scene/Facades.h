/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2008, ART+COM AG Berlin, Germany <www.artcom.de>
//
// These coded instructions, statements, and computer programs contain
// proprietary information of ART+COM AG Berlin, and are copy protected
// by law. They may be used, modified and redistributed under the terms
// of GNU General Public License referenced below. 
//    
// Alternative licensing without the obligations of the GPL is
// available upon request.
//
// GPL v3 Licensing:
//
// This file is part of the ART+COM Y60 Platform.
//
// ART+COM Y60 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ART+COM Y60 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ART+COM Y60.  If not, see <http://www.gnu.org/licenses/>.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Description: TODO  
//
// Last Review: NEVER, NOONE
//
//  review status report: (perfect, ok, fair, poor, disaster, notapplicable, unknown)
//    usefullness            : unknown
//    formatting             : unknown
//    documentation          : unknown
//    test coverage          : unknown
//    names                  : unknown
//    style guide conformance: unknown
//    technical soundness    : unknown
//    dead code              : unknown
//    readability            : unknown
//    understandabilty       : unknown
//    interfaces             : unknown
//    confidence             : unknown
//    integration            : unknown
//    dependencies           : unknown
//    cheesyness             : unknown
//
//    overall review status  : unknown
//
//    recommendations: 
//       - unknown
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/

#ifndef _scene_Facades_h_
#define _scene_Facades_h_

#include "TransformHierarchyFacade.h"
#include "LodFacade.h"
#include "Camera.h"
#include "Body.h"
#include "MaterialBase.h"
#include "Light.h"
#include "LightSource.h"
#include "Canvas.h"
#include "Viewport.h"
#include "Overlay.h"
#include "Shape.h"
#include "IncludeFacade.h"
#include "Geometry.h"
#include "Scene.h"
#include "TextureUnit.h"
#include "Texture.h"
#include "Record.h"
#include "Primitive.h"

#include <y60/image/Image.h>
#include <y60/video/Movie.h>
#include <y60/video/Capture.h>

#include <asl/dom/Nodes.h>
#include <asl/math/Matrix4.h>

namespace y60 {

    //                  theTagName           theType           theAttributeName               theDefault
    DEFINE_ATTRIBUT_TAG(SkyBoxMaterialTag,   std::string,      SKYBOX_MATERIAL_ATTRIB,   "");
    DEFINE_ATTRIBUT_TAG(LodScaleTag,         float,            LODSCALE_ATTRIB,          1);
    DEFINE_ATTRIBUT_TAG(FogModeTag,          std::string,      FOGMODE_ATTRIB,           "");
    DEFINE_ATTRIBUT_TAG(FogColorTag,         asl::Vector4f,    FOGCOLOR_ATTRIB,          asl::Vector4f(0,0,0,0));
    DEFINE_ATTRIBUT_TAG(FogRangeTag,         asl::Vector2f,    FOGRANGE_ATTRIB,          asl::Vector2f(0, 1));
    DEFINE_ATTRIBUT_TAG(FogDensityTag,       float,            FOGDENSITY_ATTRIB,        1);

    class WorldFacade :
        public TransformHierarchyFacade,
        public SkyBoxMaterialTag::Plug,
        public LodScaleTag::Plug,
        public FogModeTag::Plug,
        public FogColorTag::Plug,
        public FogRangeTag::Plug,
        public FogDensityTag::Plug
    {
        public:
            WorldFacade(dom::Node & theNode) :
                TransformHierarchyFacade(theNode),
                SkyBoxMaterialTag::Plug(theNode),
                LodScaleTag::Plug(theNode),
                FogModeTag::Plug(theNode),
                FogColorTag::Plug(theNode),
                FogRangeTag::Plug(theNode),
                FogDensityTag::Plug(theNode)
        {}
        IMPLEMENT_FACADE(WorldFacade);
    };

    typedef asl::Ptr<WorldFacade, dom::ThreadingModel> WorldFacadePtr;

    class TransformFacade :
        public TransformHierarchyFacade
    {
    public:
        TransformFacade(dom::Node & theNode) : TransformHierarchyFacade(theNode) {}
        IMPLEMENT_FACADE(TransformFacade);
    };


    //                  theTagName           theType           theAttributeName               theDefault
    DEFINE_ATTRIBUT_TAG(JointOrientationTag, asl::Quaternionf, JOINT_ORIENTATION_ATTRIB, asl::Quaternionf(0,0,0,1));

    typedef asl::Ptr<TransformFacade, dom::ThreadingModel> TransformFacadePtr;

    class JointFacade :
        public TransformHierarchyFacade,
        public JointOrientationTag::Plug
    {
    public:
        JointFacade(dom::Node & theNode) :
            TransformHierarchyFacade(theNode),
            JointOrientationTag::Plug(theNode)
        {}

        IMPLEMENT_FACADE(JointFacade);

        void registerDependenciesRegistrators() {
            TransformHierarchyFacade::registerDependenciesRegistrators();
            LocalMatrixTag::Plug::setReconnectFunction(&JointFacade::registerDependenciesForLocalMatrix);
        }

        void registerDependenciesForLocalMatrix() {
            dom::Node & myNode = getNode();
            if (myNode) {
                // local matrix
                TransformHierarchyFacade::registerDependenciesForLocalMatrix();
                LocalMatrixTag::Plug::dependsOn<JointOrientationTag>(*this);

                LocalMatrixTag::Plug::setCalculatorFunction(&JointFacade::recalculateLocalMatrix);
            }
        }

    protected:
        void recalculateLocalMatrix() {
            asl::Matrix4f myMatrix = asl::Matrix4f::Identity();
            myMatrix.scale(get<ScaleTag>());
            myMatrix.translate(-get<PivotTag>());
            asl::Matrix4f myRotation(get<OrientationTag>());
            myMatrix.postMultiply(myRotation);
            asl::Matrix4f myJointRotation(get<JointOrientationTag>());
            myMatrix.postMultiply(myJointRotation);
            myMatrix.translate(get<PositionTag>() + get<PivotTag>());
            set<LocalMatrixTag>(myMatrix);
        }
    };
    typedef asl::Ptr<JointFacade, dom::ThreadingModel> JointFacadePtr;

    inline void registerSceneFacades(dom::FacadeFactoryPtr theFactory) {
        theFactory->registerPrototype(SCENE_ROOT_NAME, dom::FacadePtr(new Scene(dom::Node::Prototype)));
        theFactory->registerPrototype(WORLD_NODE_NAME, dom::FacadePtr(new WorldFacade(dom::Node::Prototype)));
        theFactory->registerPrototype(BODY_NODE_NAME, dom::FacadePtr(new Body(dom::Node::Prototype)));
        theFactory->registerPrototype(TRANSFORM_NODE_NAME, dom::FacadePtr(new TransformFacade(dom::Node::Prototype)));
        theFactory->registerPrototype(LOD_NODE_NAME, dom::FacadePtr(new LodFacade(dom::Node::Prototype)));
        theFactory->registerPrototype(LIGHT_NODE_NAME, dom::FacadePtr(new Light(dom::Node::Prototype)));
        theFactory->registerPrototype(LIGHTSOURCE_NODE_NAME, dom::FacadePtr(new LightSource(dom::Node::Prototype)));
        theFactory->registerPrototype(PROPERTY_LIST_NAME, dom::FacadePtr(new LightPropertiesFacade(dom::Node::Prototype)),
                LIGHTSOURCE_NODE_NAME);
        theFactory->registerPrototype(CAMERA_NODE_NAME, dom::FacadePtr(new Camera(dom::Node::Prototype)));
        theFactory->registerPrototype(PROJECTOR_NODE_NAME, dom::FacadePtr(new Projector(dom::Node::Prototype)));
        theFactory->registerPrototype(JOINT_NODE_NAME, dom::FacadePtr(new JointFacade(dom::Node::Prototype)));
        theFactory->registerPrototype(CANVAS_NODE_NAME, dom::FacadePtr(new Canvas(dom::Node::Prototype)));
        theFactory->registerPrototype(VIEWPORT_NODE_NAME, dom::FacadePtr(new Viewport(dom::Node::Prototype)));
        theFactory->registerPrototype(OVERLAY_NODE_NAME, dom::FacadePtr(new Overlay(dom::Node::Prototype)));
        theFactory->registerPrototype(IMAGE_NODE_NAME, dom::FacadePtr(new Image(dom::Node::Prototype)));
        theFactory->registerPrototype(MOVIE_NODE_NAME, dom::FacadePtr(new Movie(dom::Node::Prototype)));
        theFactory->registerPrototype(CAPTURE_NODE_NAME, dom::FacadePtr(new Capture(dom::Node::Prototype)));
        theFactory->registerPrototype(TEXTURE_NODE_NAME, dom::FacadePtr(new Texture(dom::Node::Prototype)));
        theFactory->registerPrototype(TEXTUREUNIT_NODE_NAME, dom::FacadePtr(new TextureUnit(dom::Node::Prototype)));
        theFactory->registerPrototype(SHAPE_NODE_NAME, dom::FacadePtr(new Shape(dom::Node::Prototype)));
        theFactory->registerPrototype(INCLUDE_NODE_NAME, dom::FacadePtr(new IncludeFacade(dom::Node::Prototype)));
        theFactory->registerPrototype(MATERIAL_NODE_NAME, dom::FacadePtr(new MaterialBase(dom::Node::Prototype)));
        theFactory->registerPrototype(PROPERTY_LIST_NAME, dom::FacadePtr(new MaterialPropertiesFacade(dom::Node::Prototype)),
                MATERIAL_NODE_NAME);
        theFactory->registerPrototype(REQUIRES_LIST_NAME, dom::FacadePtr(new MaterialRequirementFacade(dom::Node::Prototype)),
                MATERIAL_NODE_NAME);
        theFactory->registerPrototype(RECORD_NODE_NAME, dom::FacadePtr(new RecordFacade(dom::Node::Prototype)));
        theFactory->registerPrototype(ELEMENTS_NODE_NAME, dom::FacadePtr(new Primitive(dom::Node::Prototype)));

        //=== Analytic Geometry Nodes =========================================
        theFactory->registerPrototype(PLANE_NODE_NAME, dom::FacadePtr(new Plane(dom::Node::Prototype)));
        theFactory->registerPrototype(POINT_NODE_NAME, dom::FacadePtr(new Point(dom::Node::Prototype)));
        theFactory->registerPrototype(VECTOR_NODE_NAME, dom::FacadePtr(new Vector(dom::Node::Prototype)));
    }

/*
    template <class T>
    inline asl::Ptr<T, dom::ThreadingModel>
    createFacade(dom::NodePtr theParent) {
        dom::NodePtr myNode = theParent.appendChild(dom::NodePtr(new dom::Element(T::name)));
        return myNode->getFacade<T>();
    }

    template <class Body>
    inline asl::Ptr<Body, dom::ThreadingModel>
    createFacade(dom::NodePtr theParent, const std::string & theShapeId) {
        dom::NodePtr myNode = dom::NodePtr(new dom::Element(Body::name()));
        myNode.appendAttribute(BODY_SHAPE_ATTRIB, theShapeId);
        myNode = theParent->appendChild(myNode);
        return myNode->getFacade<Body>();
    }
    */
}

#endif
