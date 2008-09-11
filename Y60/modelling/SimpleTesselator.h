//============================================================================
// Copyright (C) 2004-2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#ifndef AC_Y60_SIMPLE_TESSELATOR_FUNCTIONS_INCLUDED
#define AC_Y60_SIMPLE_TESSELATOR_FUNCTIONS_INCLUDED

#include <y60/scene/Scene.h>

#include <asl/dom/Nodes.h>
#include <asl/math/Vector234.h>
#include <asl/base/Exception.h>

#include <string>
#include <iostream>

namespace y60 {
    DEFINE_EXCEPTION(SimpleTesselatorException, asl::Exception);
   
    class SimpleTesselator {
        public:
            SimpleTesselator();
            virtual ~SimpleTesselator();
            dom::NodePtr createSurface2DFromContour(y60::ScenePtr theScene, const std::string & theMaterialId,
                                                    const VectorOfVector2f & theContour,
                                                    const std::string & theName = "Surface2DShape",
                                                    float theEqualPointsThreshold = 1e-5);
        
            // triangulate a contour/polygon, places results in STL vector
            // as series of triangles.
            bool process(const VectorOfVector2f &contour, VectorOfVector2f &result);
        
            // compute area of a contour/polygon
            float area(const VectorOfVector2f &contour);
        
            // decide if point Px/Py is inside triangle defined by
            // (Ax,Ay) (Bx,By) (Cx,Cy)
            bool insideTriangle(float Ax, float Ay,
                                float Bx, float By,
                                float Cx, float Cy,
                                float Px, float Py);
        

        private:
          bool snip(const VectorOfVector2f &contour,int u,int v,int w,int n,int *V);


    };
    typedef asl::Ptr<SimpleTesselator, dom::ThreadingModel> SimpleTesselatorPtr;

}
#endif // AC_Y60_SIMPLE_TESSELATOR_FUNCTIONS_INCLUDED

