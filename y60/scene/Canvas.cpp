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

// own header
#include "Canvas.h"

#include "Scene.h"
#include <asl/dom/Nodes.h>

using namespace std;

namespace y60 {

unsigned 
Canvas::getWidth() const { 
    asl::Ptr<IFrameBuffer> myFrameBuffer = _myFrameBuffer.lock();
    AC_TRACE << "Canvas::getWidth '" << get<NameTag>() << "' framebuffer=" << myFrameBuffer.get();
    return myFrameBuffer ? myFrameBuffer->getWidth() : 0;
}

unsigned 
Canvas::getHeight() const {
    asl::Ptr<IFrameBuffer> myFrameBuffer = _myFrameBuffer.lock();
    AC_TRACE << "Canvas::getHeight '" << get<NameTag>() << "' framebuffer=" << myFrameBuffer.get();
    return myFrameBuffer ? myFrameBuffer->getHeight() : 0;
}

bool
Canvas::setFrameBuffer(asl::Ptr<IFrameBuffer> theFrameBuffer) {
    AC_DEBUG << "Canvas::setFrameBuffer '" << get<NameTag>() << " to " << theFrameBuffer.get();
    if (theFrameBuffer) {
        asl::Ptr<IFrameBuffer> myFrameBuffer = _myFrameBuffer.lock();
        if (! myFrameBuffer) {
            //AC_TRACE << "Canvas @"<< this << ", setting framebuffer @ " << theFrameBuffer.get() << endl;
            _myFrameBuffer = theFrameBuffer;
            return true;
        } else {
            // framebuffer already set
            return false;
        } 
    } else {
        // unset framebuffer
        _myFrameBuffer = asl::Ptr<IFrameBuffer>();
        return true;
    }
}

TexturePtr
Canvas::getTarget(asl::Ptr<Scene,dom::ThreadingModel> theScene) {
    dom::NodePtr myTextureNode = theScene->getSceneDom()->getElementById( get<CanvasTargetTag>() );
    if (myTextureNode) {
        return myTextureNode->getFacade<Texture>();
    }
    return TexturePtr();
}

bool 
Canvas::hasRenderTarget() const {
    return ! get<CanvasTargetTag>().empty();
}
}
