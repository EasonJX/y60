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
//    $RCSfile: SensorServer.cpp,v $
//     $Author: ulrich $
//   $Revision: 1.8 $
//       $Date: 2005/02/03 16:59:51 $
//
// Dieters Sensor Array...
//
//=============================================================================

#include "SensorServer.h"

#include <asl/serial/SerialDeviceFactory.h>
#include <asl/base/Logger.h>
#include <sstream>
#include <memory>

using namespace std;

SensorServer::SensorServer(unsigned theComPort, unsigned theBaudRate) {
    _myComPort = asl::Ptr<asl::SerialDevice>(asl::getSerialDevice(theComPort));
    _myComPort->open(theBaudRate, 8, asl::SerialDevice::NO_PARITY, 1);
}


void
SensorServer::parseLine(const string & theLine, unsigned & theController, unsigned & theBitMask) {
    istringstream myStream(theLine);

    char  c;
    if (theLine[0] == 7) {
        myStream >> c; // skip bell
    }
    myStream >> theController;
    myStream >> c; // skip comma
    myStream >> theBitMask;
}


void
SensorServer::handleLines(string & theBuffer, SensorData & theData) {
    std::string::size_type i = theBuffer.find_first_of("\r\n");
    while(i != string::npos) {
        string myLine = theBuffer.substr(0 ,i);
        if (!myLine.empty()) {
            unsigned myController, myBitmask;
            parseLine(myLine, myController, myBitmask);
            theData.push_back(make_pair(myController, myBitmask));
        }
        theBuffer = theBuffer.substr(i+1);
        std::string::size_type nextline_start = theBuffer.find_first_not_of("\r\n");
        if ( nextline_start != string::npos) {
            theBuffer = theBuffer.substr(nextline_start);
        }
    }
}

void 
SensorServer::poll(SensorData & theData) {
    size_t myNumBytes = _myComPort->peek();
    if (myNumBytes) {
        vector<char> myBuffer(myNumBytes+1);
        _myComPort->read(&myBuffer[0], myNumBytes);
        myBuffer[myNumBytes] = 0;
        _myFifo += string(&myBuffer[0]);
        handleLines(_myFifo, theData);
    }
}
