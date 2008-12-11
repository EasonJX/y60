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

#include "VideoMsgQueue.h"

#include <asl/base/Logger.h>

using namespace asl;

namespace y60 {

VideoMsgQueue::VideoMsgQueue() {
}

VideoMsgQueue::~VideoMsgQueue() {
}

void VideoMsgQueue::push_back(VideoMsgPtr theFrame) {
    AC_DEBUG << "push_back {";
    _myListLock.lock();
    _myList.push_back(theFrame);
    _mySemaphore.post();
    _myListLock.unlock();
    AC_DEBUG << "} push_back";
}

void VideoMsgQueue::push_front(VideoMsgPtr theFrame) {
    AC_DEBUG << "push_front {";
    _myListLock.lock();
    _myList.push_front(theFrame);
    _mySemaphore.post();
    _myListLock.unlock();
    AC_DEBUG << "} push_front";
}

VideoMsgPtr VideoMsgQueue::pop_front() {
    AC_DEBUG << "pop_front {";
    _mySemaphore.wait(asl::ThreadSemaphore::WAIT_INFINITE);
    _myListLock.lock();
    VideoMsgPtr myPopper = _myList.front();
    _myList.pop_front();
    _myListLock.unlock();
    AC_DEBUG << "} pop_front";
    return myPopper;
}

VideoMsgPtr VideoMsgQueue::front() const {
    _myListLock.lock();
    VideoMsgPtr myValue = _myList.front();
    _myListLock.unlock();
    return myValue;
}

VideoMsgPtr VideoMsgQueue::back() const {
    _myListLock.lock();
    VideoMsgPtr myValue = _myList.back();
    _myListLock.unlock();
    return myValue;
}

unsigned VideoMsgQueue::size() const {
    _myListLock.lock();
    unsigned myListSize = _myList.size();
    _myListLock.unlock();
    return myListSize;
    //return _myList.size(); 
}
        
void VideoMsgQueue::close() { 
    AC_DEBUG << "VideoMsgQueue::close";
    _mySemaphore.close(); 
}

void VideoMsgQueue::reset() {
    AC_DEBUG << "VideoMsgQueue::reset";
    _mySemaphore.reset(_myList.size()); 
}

void VideoMsgQueue::clear() {
    AC_DEBUG << "VideoMsgQueue::clear";
    _mySemaphore.close(); 
    _myList.clear(); 
    _mySemaphore.reset(); 
}
void VideoMsgQueue::dump() {
    AC_DEBUG << "VideoMsgQueue::dump";
    _myListLock.lock();
    std::list<VideoMsgPtr>::iterator myItStart = _myList.begin();
    std::list<VideoMsgPtr>::iterator myItEnd = _myList.end();
    unsigned i = 0;
    for (; myItStart != myItEnd; myItStart++) {
        AC_DEBUG << "Frame in queue #" << i << " timestamp -> " <<  (*myItStart)->getTime();
        i++;
    }    
    _myListLock.unlock();
}

}

