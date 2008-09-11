/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2005, ART+COM AG Berlin, Germany
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
//   $RCSfile: IFrameBuffer.h,v $
//
//   $Revision: 1.1 $
//
//   Description:
//
//
//
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/

#ifndef _scene_IFrameBuffer_h_
#define _scene_IFrameBuffer_h_

#include <asl/base/Ptr.h>

namespace y60 {
   
class IFrameBuffer {
    public:
        virtual int getWidth() const = 0;
        virtual int getHeight() const = 0;
};

typedef asl::Ptr<IFrameBuffer> IFrameBufferPtr;

}

#endif
