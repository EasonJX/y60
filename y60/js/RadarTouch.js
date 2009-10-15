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


plug("OscReceiver");
use("spark/multitouch.js");

function RadarTouch(theEventListener) {
    this.Constructor(this, theEventListener);
}

RadarTouch.prototype.Constructor = function(Public, theEventListener) {

    const RECEIVE_TIMEOUT = 60000; //60 sec

    var _myOscReceiver = null;
    var _myLastReceivedTime = 0;
    var _myTimeoutCallback = null;
    
    var _myLastPosition = [0,0];

    var _myCursors = [];
    
    var _myEventListener = theEventListener;

    Public.Constructor = function() {
    }
    
    Public.start = function(thePort) {
        print("Starting OSC receiver on port: " + thePort);

        _myOscReceiver = new OscReceiver(thePort);
        _myOscReceiver.start();
    }
    
    Public.stop = function() {
        if (_myOscReceiver) {
            _myOscReceiver.stop();
            delete _myOscReceiver;
            _myOscReceiver = null;
        }
    }
    
    Public.registerTimeoutCallback = function(theFunc) {
        _myTimeoutCallback = theFunc;
    }
    

    //////////////////////////////////////////////////////////////////////
    // Callbacks
    //////////////////////////////////////////////////////////////////////

    Public.onOscEvent = function(theNode) {

        var myAdrStr = theNode.type;
        var myArgs = [];
        var myEventName = "";

        
        Logger.trace("osc node: " + theNode);
        
        _myLastReceivedTime = millisec();
                
        for (var i = 0; i < theNode.childNodesLength(); i++) {
            var myChild = theNode.childNode(i);
            if (myChild.nodeName == "float" || 
                myChild.nodeName == "int") {
                myArgs.push(Number(myChild.childNode("#text").nodeValue));
            } else {
                Logger.error("Unknown argument type: " + myChild.nodeName);
            }
        }

        // map multitouch events to single touch events!

        switch (myAdrStr) {
            case "add":
                if (myArgs.length == 1) {
                    var myCursor = new spark.Cursor(myArgs[0]);
                    _myCursors[myArgs[0]] = myCursor;
                    _myEventListener.dispatchEvent( new spark.CursorEvent( spark.CursorEvent.ENTER, myCursor ) ); 
                }
                break;
            case "remove":
                if (myArgs.length == 1) {
                    if (myArgs[0] in _myCursors) {
                        var myCursor = _myCursors[myArgs[0]];
                        _myEventListener.dispatchEvent( new spark.CursorEvent( spark.CursorEvent.LEAVE, myCursor ));
                        delete _myCursors[myArgs[0]];
                    }
                }
                break;
            case "move":
                if (myArgs.length == 3) {
                    var myCoords = planeToWindowCoords(myArgs[1], myArgs[2]); 
                    var myCursor = _myCursors[myArgs[0]];
                    var myProperties = {
                        position3D : new Vector3f( myCoords[0], myCoords[1], 0 ),
                        intensity  : 255
                    };
                    myCursor.update(myProperties, true);
                    _myEventListener.dispatchEvent( new spark.CursorEvent( spark.CursorEvent.MOVE, myCursor ));
                }
                break;
             case "alive":
                break;
             default:
                Logger.error("Strange osc message: " + theNode);
                break;
        }

    }
    
    Public.onFrame = function(theTime) {
        if (theTime - _myLastReceivedTime > RECEIVE_TIMEOUT) {
            Logger.warning("OSC: Input Device timed out.");
            if (_myTimeoutCallback) {
                _myTimeoutCallback();
            }
            _myLastReceivedTime = theTime;
        }        
    }

    //////////////////////////////////////////////////////////////////////
    // Private
    //////////////////////////////////////////////////////////////////////
    
    function planeToWindowCoords(theX, theY) {
        return [theX * window.width, (1-theY) * window.height];
    }

    Public.Constructor();
}
