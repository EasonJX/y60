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

function LayerManager() {
    this.Constructor(this);
}

LayerManager.prototype.Constructor = function(Public) {
    var _myLayers = [];

    //////////////////////////////////////////////////////////////////////
    // Public
    //////////////////////////////////////////////////////////////////////

    Public.setup = function(theLayerNode) {
        for(var i=0; i < theLayerNode.childNodes.length; ++i){
            var myLayerNode  = theLayerNode.childNode(i);
            var myLayerType  = myLayerNode.type;
            var myLayerName  = myLayerNode.name;
            var myLayerDepth = i*3;

            Logger.info("Setting up layer \"" + myLayerName + "\" at depth " + myLayerDepth + "...");

            use("GUI/Layers/" + myLayerType + "Layer.js");

            var myCtorCall = "new "+myLayerType + "Layer("+myLayerDepth+",'"+myLayerName+"'"+",'"+myLayerType+"')";
            var myNewLayer = eval(myCtorCall);
            myNewLayer.setup(myLayerNode);
            _myLayers.push(myNewLayer);
        }
    }

    Public.getLayerByType = function(theType){
        for (var i = 0; i< _myLayers.length; i++) {
            if (_myLayers[i].type == theType) {
                return _myLayers[i];
            }
        }
        return null;
    }

    Public.getLayerByName = function(theName) {
        for (var i=0; i<_myLayers.length; ++i) {
            if (_myLayers[i].name == theName) {
                return _myLayers[i];
            }
        }
    }

    Public.layers getter  = function() {
        return _myLayers;
    }
}
