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

#include "GenericEventSourceFilter.h"

using namespace std;
using namespace asl;
namespace y60 {
    const unsigned int GenericEventSourceFilter::MAX_CURSOR_POSITIONS_IN_HISTORY = 10;

    GenericEventSourceFilter::GenericEventSourceFilter():
        _myMaxCursorPositionsInHistory(MAX_CURSOR_POSITIONS_IN_HISTORY),
        _myFilterMultipleMovePerCursorFlag(true),
        _myMaxCursorCount(5)
    {}

    GenericEventSourceFilter::~GenericEventSourceFilter() {}

    void 
    GenericEventSourceFilter::addCursorFilter(const std::string & theEventType, const std::string & theIdAttributeName) {
        _myCursorFilter.push_back( CursorFilter(theEventType, theIdAttributeName));

    }
    // apply filter in place
    void 
    GenericEventSourceFilter::applyFilter(EventPtrList & theEventList) {
        for (unsigned i = 0; i < _myCursorFilter.size(); i++) {
            AC_DEBUG << "filter cursor events with type: " << _myCursorFilter[i]._myEventType << " and cursor id attribute: " << _myCursorFilter[i]._myCursorAttributeName;
            applyCursorFilter(_myCursorFilter[i]._myEventType, _myCursorFilter[i]._myCursorAttributeName, theEventList);
        }
    }

    bool GenericEventSourceFilter::allow2SendCursor(int theCursorId) {
        bool myAllowFlag = true;
        if (_myMaxCursorCount >-1) {
            std::vector<int>::iterator myCursorInUse = find(_myCursorsInUse.begin(), _myCursorsInUse.end(), theCursorId);    
            if (myCursorInUse == _myCursorsInUse.end()) {
                if (_myCursorsInUse.size() < _myMaxCursorCount) {
                    // we have room for cursor
                    myAllowFlag = true;
                    _myCursorsInUse.push_back(theCursorId);
                } else {
                    myAllowFlag = false;
                }
            }
        }
        return myAllowFlag;
    }

    void 
    GenericEventSourceFilter::applyCursorFilter(const std::string & theEventType, const std::string & theIdAttributeName, EventPtrList & theEventList) {
        std::map<int, GenericEventPtr > myNewestEvent;
        EventPtrList::iterator myIt = theEventList.begin();
        for (; myIt !=theEventList.end(); ) {
            GenericEventPtr myGenericEvent(dynamic_cast_Ptr<GenericEvent>(*myIt));
            dom::NodePtr myNode = myGenericEvent->getNode();
            int myCursorId = asl::as<int>(myNode->getAttributeString(theIdAttributeName));
            std::string myEventType = myNode->getAttributeString("type");
            if (myEventType == theEventType) {
                if (myNewestEvent.find(myCursorId) != myNewestEvent.end()) {
                    myIt = theEventList.erase(myIt);    
                } else {
                    ++myIt;
                }
                myNewestEvent[myCursorId] = myGenericEvent;
            } else {
                ++myIt;
            }
        }

        for (myIt = theEventList.begin(); myIt !=theEventList.end(); ++myIt) {
            GenericEventPtr myGenericEvent(dynamic_cast_Ptr<GenericEvent>(*myIt));
            dom::NodePtr myNode = myGenericEvent->getNode();
            int myCursorId = asl::as<int>(myNode->getAttributeString(theIdAttributeName));
            std::string myEventType = myNode->getAttributeString("type");
            if (myEventType == theEventType && myNewestEvent.find(myCursorId) != myNewestEvent.end()) {
                *(myIt) = myNewestEvent[myCursorId];
            }
        }
        
    }

    void 
    GenericEventSourceFilter::analyzeEvents(const EventPtrList & theEventList, const std::string & theIdAttributeName) const {
        AC_DEBUG << "GenericEventSourceFilter::analyzeEvents cursor events:";
        std::map<int, std::map<std::string, int> > myCursorEventCounter;
        {
            EventPtrList::const_iterator myEndIt   = theEventList.end();
            EventPtrList::const_iterator myIt = theEventList.begin();
            for(; myIt !=  myEndIt; ++myIt){
                GenericEventPtr myGenericEvent(dynamic_cast_Ptr<GenericEvent>(*myIt));
                dom::NodePtr myNode = myGenericEvent->getNode();
                int myCursorId = asl::as<int>(myNode->getAttributeString(theIdAttributeName));
                std::string myCursorType = myNode->getAttributeString("type");
                if (myCursorEventCounter.find(myCursorId) == myCursorEventCounter.end()) {
                    myCursorEventCounter[myCursorId][myCursorType] = 0;
                } else {
                    if (myCursorEventCounter[myCursorId].find(myCursorType) == myCursorEventCounter[myCursorId].end()) {
                        myCursorEventCounter[myCursorId][myCursorType] = 0;
                    }
                }
                myCursorEventCounter[myCursorId][myCursorType]++;
            }
        }
        {
            std::map<int, std::map<std::string, int> >::iterator myEndIt   = myCursorEventCounter.end();
            std::map<int, std::map<std::string, int> >::iterator myIt = myCursorEventCounter.begin();
            for(; myIt !=  myEndIt; ++myIt){
                std::map<std::string, int>::iterator myEndItType   = myIt->second.end();
                std::map<std::string, int>::iterator myItType      = myIt->second.begin();
                for(; myItType !=  myEndItType; ++myItType){
                    AC_DEBUG << "Cursor id: " << myIt->first << " with type: " << myItType->first << " has # " << myItType->second << " events";
                }
            }
            AC_DEBUG << "-------";
        }
    }

    void
    GenericEventSourceFilter::addPositionToHistory(const unsigned int theCursorId, const asl::Vector3f & thePosition, const unsigned long long & theTimestamp) {
        if (_myCursorPositionHistory.find(theCursorId) == _myCursorPositionHistory.end()) {
            _myCursorPositionHistory[theCursorId] = CursorPositions();
        }
        
        if (_myCursorPositionHistory[theCursorId].size() >= _myMaxCursorPositionsInHistory) {
            _myCursorPositionHistory[theCursorId].pop_front();
        }
        _myCursorPositionHistory[theCursorId].push_back(PositionInfo(thePosition,theTimestamp));
    }

    void
    GenericEventSourceFilter::addPositionToHistory(const unsigned int theCursorId, const asl::Vector2f & thePosition, const unsigned long long & theTimestamp) {
        asl::Vector3f myPosition(thePosition[0], thePosition[1], 0);
        addPositionToHistory(theCursorId, myPosition, theTimestamp);
    }

    asl::Vector3f 
    GenericEventSourceFilter::calculateAveragePosition(const unsigned int theCursorId, const asl::Vector3f & thePosition, const unsigned long long & theTimestamp) {
        addPositionToHistory(theCursorId, thePosition, theTimestamp);
        return getAveragePosition(theCursorId);
    }

    asl::Vector2f 
    GenericEventSourceFilter::calculateAveragePosition(const unsigned int theCursorId, const asl::Vector2f & thePosition, const unsigned long long & theTimestamp) {
        asl::Vector3f myPosition(thePosition[0], thePosition[1], 0);
        myPosition = calculateAveragePosition(theCursorId, myPosition, theTimestamp);
        return asl::Vector2f(myPosition[0], myPosition[1]);
    }
 
    asl::Vector3f 
    GenericEventSourceFilter::getAveragePosition(const unsigned int theCursorId) const {
        asl::Vector3f myAveragePosition = asl::Vector3f(0.0,0.0,0.0);
        unsigned int myWeight = 0;
        unsigned int myWeightCounter = 0;
        CursorPositionHistory::const_iterator myPositionsIt = _myCursorPositionHistory.find(theCursorId);
        if (myPositionsIt != _myCursorPositionHistory.end()) {
            CursorPositions::const_iterator myIt = myPositionsIt->second.begin();
            CursorPositions::const_iterator myEndIt = myPositionsIt->second.end();
            for(; myIt !=  myEndIt; ++myIt){
                myWeight++;
                myWeightCounter += myWeight;
                myAveragePosition += asl::product((myIt->_myPosition), float(myWeight));
            }
            if (myWeight > 0) {
                myAveragePosition.div(float(myWeightCounter));
            }
        }
        return myAveragePosition;
    }
 
        

    void 
    GenericEventSourceFilter::clearCursorHistoryOnRemove(const EventPtrList & theEventList) {
        EventPtrList::const_iterator myIt = theEventList.begin();
        for (; myIt !=theEventList.end(); ++myIt) {
            GenericEventPtr myGenericEvent(dynamic_cast_Ptr<GenericEvent>(*myIt));
            dom::NodePtr myNode = myGenericEvent->getNode();
            int myCursorId = asl::as<int>(myNode->getAttributeString("id"));
            std::string myEventType = myNode->getAttributeString("type");
            if (myEventType == "remove" && _myCursorPositionHistory.find(myCursorId) != _myCursorPositionHistory.end()) {
                _myCursorPositionHistory.erase(myCursorId);
            }
        }
    }
}
