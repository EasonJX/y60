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
//
//   $RCSfile: VertexArray.h,v $
//   $Author: martin $
//   $Revision: 1.2 $
//   $Date: 2005/04/21 16:25:02 $
//
//  Description: Self-Managed Memory, maybe AGP or Main Memory from a fixed size pool
//
//=============================================================================

#ifndef _ac_gl_vertex_array_h
#define _ac_gl_vertex_array_h

#include <y60/scene/VertexDataImpl.h>
#include "GLAlloc.h"

namespace y60 {

template <class T>
class VertexArray : public VertexDataImpl<T, std::vector<T, asl::GLAlloc<T> > > {
    typedef std::vector<T, asl::GLAlloc<T> > VertexDataVector;
    public:
        void lock() { 
            asl::AGPMemoryFlushSingleton::get().flushGLAGPMemory();
        }
        void unlock() {}
        void useAsPosition() const {
            glVertexPointer(3, GL_FLOAT, 0, this->getDataPtr());
            CHECK_OGL_ERROR;
        }
        void useAsNormal() const {
            glNormalPointer(GL_FLOAT, 0, this->getDataPtr());
            CHECK_OGL_ERROR;
        }
        void useAsColor() const {
            glColorPointer(4, GL_FLOAT, 0, this->getDataPtr());
            CHECK_OGL_ERROR;
        }
        void useAsTexCoord() const {
            glTexCoordPointer(this->getElementSize(), GL_FLOAT, 0, this->getDataPtr());
            CHECK_OGL_ERROR;
        }

};

}

#endif

