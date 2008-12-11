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

function Text(theParent, theTextNode, theDepth) {
    this.Constructor(this, theParent, theTextNode, theDepth);
}

Text.prototype.Constructor = function(Public, theParent, theTextNode, theDepth) {
    
    ////////////////////////////////////////
    // Member
    ////////////////////////////////////////

    var _myName   = theTextNode.name;
    var _myNode   = theTextNode;
    var _myParent = theParent;
    var _myDepth  = theDepth;

    var _myText     = null;
    var _myStyle    = null;
    var _mySize     = null;
    var _myImage    = null;
    var _myMaterial = null;
    var _myQuad     = null;
    var _myHeight = 0;
    var _myWidth  = 0;

    ////////////////////////////////////////
    // Public
    ////////////////////////////////////////

    Public.text getter = function() {
        return _myText;
    }

    Public.text setter = function(theText) {
        _myText = theText;
        draw();
    }

    Public.style getter = function() {
        return _myStyle;
    }

    Public.style setter = function(theStyle) {
        _myStyle = theStyle;
        draw();
    }

    Public.bodies getter = function() {
        return [_myQuad.body];
    }
    
    Public.body getter = function() {
        return _myQuad.body;
    }

    Public.name getter = function() {
        return _myName;
    }

    Public.color setter = function(theColor) {
        _myStyle.textColor = theColor;
    }
    
    Public.visible getter = function() {
        return Public.body.visible;
    }
    
    Public.visible setter = function(theVisibility) {
        Public.body.visible = theVisibility;
    }
    
    Public.alpha getter = function() {
        return _myMaterial.properties.surfacecolor[3];
    }

    Public.alpha setter = function(theAlpha) {
        Modelling.setAlpha(_myQuad.body, theAlpha);
    }
    
    Public.width getter = function() {
        return _myWidth;
    }
    
    Public.width setter = function(theWidth) {
        _myWidth = theWidth;
    }
    
    Public.height getter = function() {
        return _myHeight;
    }
    
    Public.height setter = function(theHeight) {
        _myHeight = theHeight;
    }

    ////////////////////////////////////////
    // setup
    ////////////////////////////////////////

    function draw() {
        textToImage(_myImage, _myText, _myStyle, _mySize);
    }
        
    function setup() {
        Logger.info("Setting up text " + _myName);

        if("text" in _myNode) {
            _myText = _myNode.text;
        } else {
            _myText = "";
        }
        
        if("z" in _myNode) {
            _myDepth += Number(_myNode.z);
        }
        
        _myWidth = _myNode.width;
        _myHeight = _myNode.height;

        _myStyle = fontStyleFromNode(_myNode);
        _mySize = new Vector2f(_myWidth, _myHeight);

        _myImage = textAsImage(_myText, _myStyle, _mySize);

        _myMaterial = Modelling.createUnlitTexturedMaterial(window.scene, _myImage);
        _myMaterial.transparent = true;

        _myQuad = createQuad(_myParent, _myName,
                             new Vector2f(_myNode.width,_myNode.height),
                             new Vector3f(_myNode.x,_myNode.y,_myDepth),
                             _myMaterial,
                             /* insensible */ true,
                             /* visible */    true);
    }
    
    setup();
}
