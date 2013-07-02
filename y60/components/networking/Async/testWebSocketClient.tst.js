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

/*globals use, plug, UnitTest, millisec, msleep, Async, ENSURE, SUCCESS, FAILURE, gc, UnitTestSuite,
          print, exit, RequestManager, Request, Logger */

use("UnitTest.js");

plug("NetAsync");

var TIMEOUT = 1000;

function WebSocketUnitTest() {
    this.Constructor(this, "WebSocketUnitTest");
}

WebSocketUnitTest.prototype.Constructor = function (obj, theName) {

    UnitTest.prototype.Constructor(obj, theName);

    function testWebSocket() {

        var done = true;
        Logger.info("creating client");
        var websocket = new Async.WebSocket("ws://echo.websocket.org");
        websocket.onopen = function(evt) {};
        websocket.onclose = function(evt) {};
        websocket.onmessage = function(evt) {};
        websocket.onerror = function(evt) {};

        websocket = null;
        gc();

        while (true) {
            Async.onFrame();
            gc();
            msleep(50);
            if (done) {
                break;
            }
        }
    }

    obj.run = function () {
        testWebSocket();
        gc();
    };

};

var myTestName = "testWebSocket.tst.js";
var mySuite = new UnitTestSuite(myTestName);

mySuite.addTest(new WebSocketUnitTest());
mySuite.run();

print(">> Finished test suite '" + myTestName + "', return status = " + mySuite.returnStatus() + ".");
exit(mySuite.returnStatus());
