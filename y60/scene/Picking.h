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

*/

#ifndef _scene_Picking_h_
#define _scene_Picking_h_

#include <asl/dom/Nodes.h>

namespace asl {
  template <class T> class LineSegment;
};

namespace y60 {
    class Camera;
    class Viewport;
    struct IntersectionInfo;
    struct CollisionInfo;
    typedef asl::Ptr<y60::Viewport, dom::ThreadingModel> ViewportPtr;
    typedef std::vector<y60::IntersectionInfo> IntersectionInfoVector;
    typedef std::vector<y60::CollisionInfo> CollisionInfoVector;
    typedef asl::Ptr<Camera, dom::ThreadingModel> CameraPtr;

    class Picking {
    public:
        dom::NodePtr pickBody(const dom::Node & theViewportNode, const unsigned int theScreenPixelX, const unsigned int theScreenPixelY) const;
        dom::NodePtr pickBodyBySweepingSphereFromBodies(const dom::Node & theViewportNode, const unsigned int theScreenPixelX, const unsigned int theScreenPixelY, const float theSphereRadius) const;
        y60::IntersectionInfo getPickedBodyInformation(const dom::Node & theViewportNode, const unsigned int theScreenPixelX, const unsigned int theScreenPixelY) const;
        y60::IntersectionInfoVector getPickedBodiesInformation(const dom::Node & theViewportNode, const unsigned int theScreenPixelX, const unsigned int theScreenPixelY) const;

    private:
        void getLineSegmentAndWorldNode(const dom::Node & theViewportNode, const unsigned int theScreenPixelX, const unsigned int theScreenPixelY, asl::LineSegment<float> & theLineSegment, dom::NodePtr & theWoldNode) const; 
        y60::IntersectionInfoVector findAllIntersectionsInformation(const dom::NodePtr theRootNode, const asl::LineSegment<float> & theLineSegment) const;
        y60::IntersectionInfo findNearestIntersectionInformation(const dom::NodePtr theRootNode, const asl::LineSegment<float> & theLineSegment) const;
        y60::CollisionInfoVector pickCollisionsBySweepingSphereFromBodies(const CameraPtr theCamera, const dom::Node & theViewportNode, const unsigned int theScreenPixelX, const unsigned int theScreenPixelY, const float theSphereRadius, const dom::NodePtr theRootNode) const; 
        void getNearAndFarPlanePos(const CameraPtr theCamera, const dom::Node & theViewportNode, const unsigned int theScreenPixelX, const unsigned int theScreenPixelY, asl::Point3f& theNearClipPos, asl::Point3f& theFarClipPos ) const;
        void transformClipToWorld(asl::Point3f & theNearClipPos, asl::Point3f & theFarClipPos, const CameraPtr theCamera) const;

    };
    typedef asl::Ptr<Picking, dom::ThreadingModel> PickingPtr;
}

#endif
