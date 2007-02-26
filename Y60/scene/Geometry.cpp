//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: Geometry.cpp,v $
//   $Author: pavel $
//   $Revision: 1.3 $
//   $Date: 2005/04/18 19:59:30 $
//
//  Description: Geometry class.
//
//=============================================================================

#include "Geometry.h"
#include "Scene.h"

#include <asl/Logger.h>
#include <asl/Matrix4.h>
#include <asl/GeometryUtils.h>
#include <asl/linearAlgebra.h>

using namespace std;
using namespace asl;

namespace y60 {


    //=== Plane ============================================================
    Plane::Plane(dom::Node & theNode) : GeometryBase( theNode ) {}
    Plane::~Plane() { AC_TRACE << "Plane DTOR " << this << endl; }
    
    void
    Plane::recalculateBoundingBox() {  
        if (get<VisibleTag>()) {

            Box3f myBoundingBox;
            myBoundingBox.makeEmpty(); // makeFull would be more correct but would break alot of other things,
                                       // e.g. world size would always be infinity
            TransformHierarchyFacade::recalculateBoundingBox();
            myBoundingBox.extendBy(get<BoundingBoxTag>());

            set<BoundingBoxTag>(myBoundingBox);

            // planes are not cullable
            set<CullableTag>(false);
        }
    }

    void
    Plane::recalculateGlobalPrimitive() {
        set<GlobalPlaneTag>(this->get<PlaneTag>() * this->get<GlobalMatrixTag>());
    }

    //=== Point ============================================================
    Point::Point(dom::Node & theNode) : GeometryBase( theNode ) {};
    Point::~Point() {
        AC_TRACE << "Point DTOR " << this << endl;
    }
    
    void
    Point::recalculateBoundingBox() {  
        if (get<VisibleTag>()) {
            // TODO: extend box by geometric primitives that have a valid box 
            //       ... like sphere, box, point ... plane doesn't have one ...
            Box3f myBoundingBox;
            myBoundingBox.makeEmpty();

            TransformHierarchyFacade::recalculateBoundingBox();
            myBoundingBox.extendBy(get<BoundingBoxTag>());

            myBoundingBox.extendBy( get<GlobalPointTag>() );

            //AC_PRINT << "recalculateBoundingBox point";
            set<BoundingBoxTag>(myBoundingBox);
        }
    }

    void
    Point::recalculateGlobalPrimitive() {
        set<GlobalPointTag>(get<PointTag>() * get<GlobalMatrixTag>());
    }

    //=== Vector ============================================================
    Vector::Vector(dom::Node & theNode) : GeometryBase( theNode ) {};
    Vector::~Vector() {
        AC_TRACE << "Vector DTOR " << this << endl;
    }
    
    void
    Vector::recalculateBoundingBox() {  
        if (get<VisibleTag>()) {
            // TODO: extend box by geometric primitives that have a valid box 
            //       ... like sphere, box, point ... plane doesn't have one ...
            Box3f myBoundingBox;
            myBoundingBox.makeEmpty();

            TransformHierarchyFacade::recalculateBoundingBox();
            myBoundingBox.extendBy(get<BoundingBoxTag>());

            Vector3f myTranslation = get<GlobalMatrixTag>().getTranslation();
            myBoundingBox.extendBy( myTranslation );
            myBoundingBox.extendBy( myTranslation + get<GlobalVectorTag>() );

            //AC_PRINT << "recalculateBoundingBox point";
            set<BoundingBoxTag>(myBoundingBox);
        }
    }

    void
    Vector::recalculateGlobalPrimitive() {
        set<GlobalVectorTag>( asVector( asPoint( get<VectorTag>()) *  get<GlobalMatrixTag>() ) );
    }

    //=== LineSegment ============================================================
    LineSegment::LineSegment(dom::Node & theNode) : GeometryBase( theNode ) {};
    LineSegment::~LineSegment() {
        AC_TRACE << "LineSegment DTOR " << this << endl;
    }
    
    void
    LineSegment::recalculateBoundingBox() {  
        if (get<VisibleTag>()) {
            // TODO: extend box by geometric primitives that have a valid box 
            //       ... like sphere, box, point ... plane doesn't have one ...
            Box3f myBoundingBox;
            myBoundingBox.makeEmpty();

            TransformHierarchyFacade::recalculateBoundingBox();
            myBoundingBox.extendBy(get<BoundingBoxTag>());

            const asl::LineSegment<float> & myLineSegment = get<GlobalLineSegmentTag>();
            myBoundingBox.extendBy( myLineSegment.origin );
            myBoundingBox.extendBy( myLineSegment.end );

            //AC_PRINT << "recalculateBoundingBox point";
            set<BoundingBoxTag>(myBoundingBox);
        }
    }

    void
    LineSegment::recalculateGlobalPrimitive() {
        set<GlobalLineSegmentTag>( get<LineSegmentTag>() *  get<GlobalMatrixTag>() );
    }

    //=== Geometry Baseclass ===============================================
    template <class PRIMITIVE, class PRIMITIVE_TAG, class GLOBAL_PRIMITIVE_TAG, class DERIVED_FACADE>
    Geometry<PRIMITIVE, PRIMITIVE_TAG, GLOBAL_PRIMITIVE_TAG, DERIVED_FACADE>::
    Geometry( dom::Node & theNode) : 
            TransformHierarchyFacade( theNode ),
            PRIMITIVE_TAG::Plug( theNode ),
            dom::FacadeAttributePlug<GLOBAL_PRIMITIVE_TAG>( this ),
            ColorTag::Plug( theNode ),
            AG::LineWidthTag::Plug( theNode ),
            AG::PointSizeTag::Plug( theNode )
    {}

    template <class PRIMITIVE, class PRIMITIVE_TAG, class GLOBAL_PRIMITIVE_TAG, class DERIVED_FACADE>
    Geometry<PRIMITIVE, PRIMITIVE_TAG, GLOBAL_PRIMITIVE_TAG, DERIVED_FACADE>::
    ~Geometry() {}

    template <class PRIMITIVE, class PRIMITIVE_TAG, class GLOBAL_PRIMITIVE_TAG, class DERIVED_FACADE>
    asl::Ptr<DERIVED_FACADE, dom::ThreadingModel> 
    Geometry<PRIMITIVE, PRIMITIVE_TAG, GLOBAL_PRIMITIVE_TAG, DERIVED_FACADE>::
    create(dom::NodePtr theParent, const PRIMITIVE & thePrimitive) {
        dom::NodePtr myNode = dom::NodePtr(new dom::Element( DERIVED_FACADE::getNodeName() ));
        myNode->appendAttribute(GEOMETRY_VALUE_ATTRIB, thePrimitive);
        myNode = theParent->appendChild(myNode);
        return myNode->getFacade<DERIVED_FACADE>();
    }

    template <class PRIMITIVE, class PRIMITIVE_TAG, class GLOBAL_PRIMITIVE_TAG, class DERIVED_FACADE>
    void 
    Geometry<PRIMITIVE, PRIMITIVE_TAG, GLOBAL_PRIMITIVE_TAG, DERIVED_FACADE>::
    registerDependenciesRegistrators() {
        TransformHierarchyFacade::registerDependenciesRegistrators();
        BoundingBoxTag::Plug::setReconnectFunction( & DERIVED_FACADE::registerDependenciesForBoundingBox);
        GLOBAL_PRIMITIVE_TAG::Plug::setReconnectFunction( & DERIVED_FACADE::registerDependenciesForGlobalPrimitive);
    }

    template <class PRIMITIVE, class PRIMITIVE_TAG, class GLOBAL_PRIMITIVE_TAG, class DERIVED_FACADE>
    void 
    Geometry<PRIMITIVE, PRIMITIVE_TAG, GLOBAL_PRIMITIVE_TAG, DERIVED_FACADE>::
    registerDependenciesForGlobalPrimitive() {
        if (getNode()) {
            GLOBAL_PRIMITIVE_TAG::Plug::template dependsOn<PRIMITIVE_TAG>(*this);
            GLOBAL_PRIMITIVE_TAG::Plug::template dependsOn<GlobalMatrixTag>(*this);

            asl::Ptr<DERIVED_FACADE, dom::ThreadingModel> mySelf = dynamic_cast_Ptr<DERIVED_FACADE>(getSelf());
            GLOBAL_PRIMITIVE_TAG::Plug::getValuePtr()->dom::Field::template setCalculatorFunction(
                    mySelf, & DERIVED_FACADE::recalculateGlobalPrimitive);
        }
    }

    template <class PRIMITIVE, class PRIMITIVE_TAG, class GLOBAL_PRIMITIVE_TAG, class DERIVED_FACADE>
    void 
    Geometry<PRIMITIVE, PRIMITIVE_TAG, GLOBAL_PRIMITIVE_TAG, DERIVED_FACADE>::
    registerDependenciesForBoundingBox() {
        if (getNode()) {
            TransformHierarchyFacade::registerDependenciesForBoundingBox();

            BoundingBoxTag::Plug::dependsOn<GLOBAL_PRIMITIVE_TAG>(*this);

            asl::Ptr<DERIVED_FACADE, dom::ThreadingModel> mySelf = dynamic_cast_Ptr<DERIVED_FACADE>(getSelf());
            BoundingBoxTag::Plug::getValuePtr()->setCalculatorFunction(mySelf, & DERIVED_FACADE::recalculateBoundingBox);
        }
    }
}




