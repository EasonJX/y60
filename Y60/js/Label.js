//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

///////////////////////////////////////////////////////////////////////////////////////////
//
//  Label Style properties:
//
//  string              font          (Path to font)
//  int                 fontsize      (Size of font)
//  Vector4f            textColor
//  enum                HTextAlign    (Renderer.CENTER_ALIGNMENT | LEFT_ALIGNMENT | RIGHT_ALIGNMENT)
//  enum                VTextAlign    (Renderer.TOP_ALIGNMENT | CENTER_ALIGNMENT | BOTTOM_ALIGNMENT)
//  Vector4f            color
//  int                 topPad        (optional)
//  int                 bottomPad     (optional)
//  int                 rightPad      (optional)
//  int                 leftPad       (optional)
//  float               tracking      (optional, default 0)
//
///////////////////////////////////////////////////////////////////////////////////////////

use("Overlay.js");

var ourFontCache = [];

function LabelBase(Public, Protected, theScene,
                   theSize, thePosition, theStyle, theParent)
{
    ///////////////////////////////////////////////////////////////////////////////////////////
    // Inheritance
    ///////////////////////////////////////////////////////////////////////////////////////////
    ImageOverlayBase(Public, Protected, theScene, null, thePosition, theParent);

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Public
    ///////////////////////////////////////////////////////////////////////////////////////////

    Public.setText = function(theText, theStyle) {

        var myStyle = null;
        if (theStyle) {
            myStyle = theStyle;
        } else {
            myStyle = Protected.style;
        }

        var topPad    = "topPad"    in myStyle ? myStyle.topPad : 0;
        var bottomPad = "bottomPad" in myStyle ? myStyle.bottomPad : 0;
        var rightPad  = "rightPad"  in myStyle ? myStyle.rightPad : 0;
        var leftPad   = "leftPad"   in myStyle ? myStyle.leftPad : 0;
        var tracking  = "tracking"  in myStyle ? myStyle.tracking : 0;
        window.setTextStyle(Renderer.BLENDED_TEXT);
        window.setTextPadding(topPad, bottomPad, leftPad, rightPad);
        window.setHTextAlignment(myStyle.HTextAlign);
        window.setVTextAlignment(myStyle.VTextAlign);
        window.setTextColor(myStyle.textColor, new Vector4f(1,1,1,1));
        window.setTracking(tracking);

        var myFontName = myStyle.font + "_" + myStyle.fontsize;
        if (!(myFontName in ourFontCache)) {
            window.loadTTF(myFontName, myStyle.font, myStyle.fontsize);
            ourFontCache[myFontName] = true;
        }

        var myImage = Protected.getImageNode();
        var mySize = window.renderTextAsImage(myImage, theText, myFontName, Public.width, Public.height);

        Public.srcsize.x = mySize[0] / myImage.width;
        Public.srcsize.y = mySize[1] / myImage.height;
        window.setHTextAlignment(Renderer.LEFT_ALIGNMENT);
        window.setVTextAlignment(Renderer.TOP_ALIGNMENT);
        window.setTextPadding(0,0,0,0);

        if (theSize == null) {
            Public.width = mySize[0];
            Public.height = mySize[1];
        } else if (theSize[0] == 0) {
            Public.width = mySize[0];
        } else if (theSize[1] == 0) {
            Public.height = mySize[1];
        }
    }

    Public.setImage = function(theSource) {
        var myImage = Protected.getImageNode();
        myImage.src = theSource;
        window.scene.update(Scene.IMAGES);
        var mySize = getImageSize(myImage);
        Public.width  = mySize.x;
        Public.height = mySize.y;
        Public.srcsize.x = 1;
        Public.srcsize.y = 1;
    }
/*
    Public.setBackgroundTransparent = function() {
        Public.textures.lastChild.applymode = "modulate";
    }
*/
    ///////////////////////////////////////////////////////////////////////////////////////////
    // Protected
    ///////////////////////////////////////////////////////////////////////////////////////////

    Protected.style = theStyle;

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Private
    ///////////////////////////////////////////////////////////////////////////////////////////

    Protected.getImageNode= function() {
        if (Public.image == null) {
            var myImage = theScene.images.appendChild(new Node("<image/>").firstChild);
            myImage.resize = "pad";
            Protected.addImage(myImage);
        }
        return Public.image;
    }

    function setup() {
        Public.color = Protected.style.color;
        if (theSize) {
            Public.width  = theSize[0];
            Public.height = theSize[1];
        }
        Public.position.x = thePosition[0];
        Public.position.y = thePosition[1];
    }

    setup();
}

function Label(theScene, theText, theSize, thePosition, theStyle, theParent) {
    var Public    = this;
    var Protected = {}
    LabelBase(Public, Protected, theScene, theSize, thePosition, theStyle, theParent);
    Public.setText(theText);
}

function BackgroundImageLabel(theScene, theText, theBackgroundImageSrc, theSize,
    thePosition, theStyle, theParent)
{
    var Public    = this;
    var Protected = {}
    LabelBase(Public, Protected, theScene, theSize, thePosition, theStyle, theParent);

    Protected.getImageNode = function() {
        if (Public.image == null) {
            Protected.addImage(theBackgroundImageSrc);
            var myImage = theScene.images.appendChild(new Node("<image/>").firstChild);
            myImage.resize = "pad";
            Protected.addImage(myImage);
        }
        return Public.images[Public.images.length - 1];
    }

    Public.setText(theText);

    Public.setBackgroundImage = function(theImageSrc) {
        Public.image.src = theImageSrc;
        //Public.image = theScene.createImage(theImageSrc);
    }
}