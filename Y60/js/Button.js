//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: Button.js,v $
//   $Author: christian $
//   $Revision: 1.21 $
//   $Date: 2004/07/28 09:16:54 $
//
//
//=============================================================================

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Button Style properties:
//
//  string              font          (Path to font)
//  int                 fontsize      (Size of font)
//  Vector4f            textColor
//  enum                HTextAlign    (Renderer.CENTER_ALIGNMENT | LEFT_ALIGNMENT | RIGHT_ALIGNMENT)
//  enum                VTextAlign    (Renderer.TOP_ALIGNMENT | CENTER_ALIGNMENT | BOTTOM_ALIGNMENT)
//  Vector4f            color
//  Vector4f            selectedColor
//  int                 topPad        (optional)
//  int                 bottomPad     (optional)
//  int                 rightPad      (optional)
//  int                 leftPad       (optional)
//
///////////////////////////////////////////////////////////////////////////////////////////

use("Label.js");

///////////////////////////////////////////////////////////////////////////////////////////
//
// Button Base
//
///////////////////////////////////////////////////////////////////////////////////////////

var ourButtonCounter = 0;

const MOUSE_UP   = 0;
const MOUSE_DOWN = 1;

function ButtonBase(Public, Protected, theSceneViewer, theId, theSize, thePosition, theStyle) {

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Inheritance
    ///////////////////////////////////////////////////////////////////////////////////////////

    LabelBase(Public, Protected, theSceneViewer, theSize, thePosition, theStyle);

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Public
    ///////////////////////////////////////////////////////////////////////////////////////////

    Public.onClick = function(theButton) {
        print ("Clicked " + Public.name + " - Override me!");
    }

    Public.isPressed = function(theButton) {
        return _isPressed;
    }

    Public.setPressed = function(theFlag) {
        if (theFlag) {
            Public.color = Protected.myStyle.selectedColor;
        } else {
            Public.color = Protected.myStyle.color;
        }
        _isPressed = theFlag;
    }    

    Public.getId = function() {
        return Protected.id;
    }

    Public.onMouseButton = function(theState, theX, theY) {
        if (Public.enabled && Public.visible) {
            if (theState == MOUSE_UP && _isPressed) {
                Public.setPressed(false);
                Public.onClick(this);
            } else if (theState == MOUSE_DOWN && !_isPressed && Public.touches(theX, theY)) {
                Public.setPressed(true);
            }
        }
    }

    Public.enabled = true;

    Public.setToggleGroup = function(theButtons) {
        // Replace the onMouseButton function with something more advanced
        Public.onMouseButton = function(theState, theX, theY) {
            if (Public.enabled && Public.visible && theState == MOUSE_DOWN && !_isPressed && Public.touches(theX, theY)) {
                for (var i = 0; i < theButtons.length; ++i) {
                    if (theButtons[i].isPressed()) {
                        theButtons[i].setPressed(false);
                    }
                }
                Public.setPressed(true);
                Public.onClick(this);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Protected
    ///////////////////////////////////////////////////////////////////////////////////////////

    Protected.id = theId;

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Private
    ///////////////////////////////////////////////////////////////////////////////////////////

    var _isPressed     = false;
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// TextButton
//
///////////////////////////////////////////////////////////////////////////////////////////

function TextButton(theSceneViewer, theId, theText, theSize, thePosition, theStyle) {
    var Public    = this;
    var Protected = {}
    TextButtonBase(Public, Protected, theSceneViewer, theId, theText, theSize, thePosition, theStyle);
}

function TextButtonBase(Public, Protected, theSceneViewer, theId, theText, theSize, thePosition, theStyle) {
    ButtonBase(Public, Protected, theSceneViewer, theId, theSize, thePosition, theStyle);
    Public.setText(theText);
}

///////////////////////////////////////////////////////////////////////////////////////////
//
// ImageButton
//
///////////////////////////////////////////////////////////////////////////////////////////

function ImageButton(theSceneViewer, theId, theSource, thePosition, theStyle) {
    var Public    = this;
    var Protected = {}
    ButtonBase(Public, Protected, theSceneViewer, theId, [1,1], thePosition, theStyle);
    Public.setImage(theSource);
}

function DualImageButton(theSceneViewer, theId, theSources, thePosition, theStyle) {
    var Base      = {};
    var Public    = this;
    var Protected = {};
    ButtonBase(Public, Protected, theSceneViewer, theId, [1,1], thePosition, theStyle);
    Public.setImage(theSources[0]);

    Base.setPressed = Public.setPressed;
    Public.setPressed = function(theFlag) {
        if (theFlag) {
            Public.color = Protected.myStyle.selectedColor;
            Public.setImage(theSources[1]);
        } else {
            Public.color = Protected.myStyle.color;
            Public.setImage(theSources[0]);
        }
        Base.setPressed(theFlag);
    }
}
