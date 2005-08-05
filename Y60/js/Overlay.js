//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy Protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: OverlayManager.js,v $
//   $Author: martin $
//   $Revision: 1.43 $
//   $Date: 2005/04/29 13:37:56 $
//
//
//=============================================================================

var ourOverlayCounter = 0;

// Pure virtual base class
function OverlayBase(Public, Protected, theManager, thePosition, theParent) {

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Convenience access to referenced nodes
    ///////////////////////////////////////////////////////////////////////////////////////////

    Public.toString = function() {
        return _myNode.toString();
    }

    Public.material getter = function() {
        return _myMaterial;
    }
    Public.material setter = function(theMaterial) {
        _myNode.material = theMaterial.id;
        _myMaterial      = theMaterial;
        _myColor         = getDescendantByName(_myMaterial, "surfacecolor", true).firstChild;

        // Notify derived classes
        Protected.onMaterialChange();
    }

    Public.color getter = function() {
        return _myColor.nodeValue;
    }

    Public.color setter = function(theColor) {
        _myColor.nodeValue = theColor;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Delegation of all overlay node attributes
    ///////////////////////////////////////////////////////////////////////////////////////////

    Public.name getter = function() { return _myNode.name; }
    Public.name setter = function(theArgument) { _myNode.name = theArgument; }

    Public.visible getter = function() { return _myNode.visible; }
    Public.visible setter = function(theArgument) { _myNode.visible = theArgument; }

    Public.position getter = function() { return _myNode.position; }
    Public.position setter = function(theArgument) { _myNode.position = theArgument; }

    Public.srcorigin getter = function() { return _myNode.srcorigin; }
    Public.srcorigin setter = function(theArgument) { _myNode.srcorigin = theArgument; }

    Public.srcsize getter = function() { return _myNode.srcsize; }
    Public.srcsize setter = function(theArgument) { _myNode.srcsize = theArgument; }

    Public.alpha getter = function() { return _myNode.alpha; }
    Public.alpha setter = function(theArgument) { _myNode.alpha = theArgument; }

    Public.width getter = function() { return _myNode.width; }
    Public.width setter = function(theArgument) { _myNode.width = theArgument; }

    Public.height getter = function() { return _myNode.height; }
    Public.height setter = function(theArgument) { _myNode.height = theArgument; }

    Public.bordercolor getter = function() { return _myNode.bordercolor; }
    Public.bordercolor setter = function(theArgument) { _myNode.bordercolor = theArgument; }

    Public.borderwidth getter = function() { return _myNode.borderwidth; }
    Public.borderwidth setter = function(theArgument) { _myNode.borderwidth = theArgument; }

    Public.topborder getter = function() { return _myNode.topborder; }
    Public.topborder setter = function(theArgument) { _myNode.topborder = theArgument; }

    Public.bottomborder getter = function() { return _myNode.bottomborder; }
    Public.bottomborder setter = function(theArgument) { _myNode.bottomborder = theArgument; }

    Public.leftborder getter = function() { return _myNode.leftborder; }
    Public.leftborder setter = function(theArgument) { _myNode.leftborder = theArgument; }

    Public.rightborder getter = function() { return _myNode.rightborder; }
    Public.rightborder setter = function(theArgument) { _myNode.rightborder = theArgument; }

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Utility functions
    ///////////////////////////////////////////////////////////////////////////////////////////

    /// Removes the overlay and the connected material and image node from the scene dom
    Public.removeFromScene = function() {
        _myNode.parentNode.removeChild(_myNode);
        _myMaterial.parentNode.removeChild(_myMaterial);
    }

    /// Moves overlay to first position in the overlay z-order
    Public.moveToTop = function() {
        var myParent = _myNode.parentNode;
        myParent.removeChild(_myNode);
        myParent.appendChild(_myNode);
    }

    /// This checks if the square defined by theX/theY and theSquareSize overlaps with the overlay
    //  @param theXPos        integer   X-Center of the square to check for overlap
    //  @param theXPos        integer   Y-Center of the square to check for overlap
    //  @param theSquareSize  Array[2]  Size of the square to check for overlay (optional, default is [1,1])
    //  @rval  true, if overlay overlaps with the square, false, if it does not
    Public.touches = function(theXPos, theYPos, theSquareSize) {
        var myLeftPixel = _myNode.position.x;
        var myTopPixel  = _myNode.position.y;
        if (theSquareSize != undefined) {
            var myOverlayBox = new Box3f([myLeftPixel, myTopPixel, 0],
                [myLeftPixel + _myNode.width - 1, myTopPixel + _myNode.height - 1, 1]);
            var myMouseBox = new Box3f([theXPos - theSquareSize, theYPos - theSquareSize, 0],
                [theXPos + theSquareSize, theYPos + theSquareSize, 1]);
            return myMouseBox.intersects(myOverlayBox);
        } else {
            if (myLeftPixel <= theXPos && myTopPixel <= theYPos &&
                myLeftPixel + _myNode.width > theXPos && myTopPixel  + _myNode.height > theYPos)
            {
                return true;
            }
        }
        return false;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Protected
    ///////////////////////////////////////////////////////////////////////////////////////////

    /// Overload this function to get notified on material changes
    Protected.onMaterialChange = function() {}

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Private
    ///////////////////////////////////////////////////////////////////////////////////////////

    /// Create an untextured overlay
    function setup() {
        var myName = "Overlay_" + ourOverlayCounter++;

        var myMaterialId = createUniqueId();
        var myMaterialString =
        '<material name="' + myName + 'M" id="' + myMaterialId + '" transparent="1" >\n' +
        '    <properties><vector4f name="surfacecolor">[1,1,1,1]</vector4f></properties>\n' +
        '    <requires><feature class="lighting" values="[10[unlit]]"/></requires>\n' +
        '</material>';
        _myMaterial = new Node(myMaterialString).firstChild;
        theManager.materials.appendChild(_myMaterial);
        _myColor = getDescendantByName(_myMaterial, "surfacecolor", true).firstChild;

        var myParent = theParent ? theParent : theManager.overlays;
        var myOverlayString = '<overlay name="' + myName + '" material="' + _myMaterial.id + '"/>';
        var myNode = new Node(myOverlayString);
        _myNode = myParent.appendChild(myNode.firstChild);

        if (thePosition) {
            _myNode.position.x = thePosition[0];
            _myNode.position.y = thePosition[1];
        }
    }

    var _myNode     = null;
    var _myMaterial = null;
    var _myColor    = null;

    setup();
}

/// Creates a uniform colored overlay
// @param theManager   Object    The overlay manager for the used viewport
// @param theColor     Vector4f  Color of the overlay
// @param thePosition  Vector2f  Pixelposition of the overlay (optional, default is [0,0])
// @param theSize      Vector2f  Size of the overlay (optional, default is [1,1]
// @param theParent    Node      Parent overlay node (optional, optional default is toplevel)
function Overlay(theManager, theColor, thePosition, theSize, theParent) {
    var Public    = this;
    var Protected = {};
    OverlayBase(Public, Protected, theManager, thePosition, theParent);
    if (theSize) {
        Public.width  = theSize[0];
        Public.height = theSize[1];
    }

    if (theColor) {
        Public.color = theColor;
    }
}

// Pure virtual base class
function TextureOverlay(Public, Protected, theManager, thePosition, theParent) {
    var Base = {}
    OverlayBase(Public, Protected, theManager, thePosition, theParent);

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Public
    ///////////////////////////////////////////////////////////////////////////////////////////

    Public.image getter = function() {
        return Protected.myImage;
    }
    Public.image setter = function(theImage) {
        Protected.myImage = theImage;
        Public.texture.image = theImage.id;
    }

    Public.texture getter = function() {
        return _myTexture;
    }

    Public.texture setter = function(theTexture) {
        _myTexture = theTexture;
        _myTextures.replaceChild(theTexture, _myTextures.firstChild);
        Protected.onTextureChange();
    }

    Base.removeFromScene = Public.removeFromScene;
    Public.removeFromScene = function() {
        Protected.myImage.parentNode.removeChild(Protected.myImage);
        Base.removeFromScene();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Protected
    ///////////////////////////////////////////////////////////////////////////////////////////

    Protected.onTextureChange = function() {
        Protected.myImage = Public.texture.getElementById(Public.texture.image);
        if (Protected.myImage == null) {
            throw new Exception("Could not find image with id: " + Public.texture.image, fileline());
        }
    }

    Protected.onMaterialChange = function() {
        _myTextures = getDescendantByTagName(Public.material, "textures", false);
        if (_myTextures.childNodes.length != 1) {
            throw new Exception("TextureOverlay can only have one texture, but it has: " + _myTextures.childNodes.length, fileline());
        }

        _myTexture = _myTextures.firstChild;
        Protected.onTextureChange();
    }

    Protected.addTexture = function(theImageId) {
        _myTextures = getDescendantByTagName(Public.material, "textures", false);
        if (!_myTextures) {
            _myTextures = Public.material.appendChild(Node.createElement("textures"));
        }
        _myTexture  = _myTextures.appendChild(Node.createElement("texture"));
        _myTexture.applymode = "modulate";
        _myTexture.wrapmode  = "repeat";
        _myTexture.image     = theImageId;
        addTextureRequirements(_myTextures.childNodesLength());
        return _myTexture;
    }

    Protected.myImage = null;

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Private
    ///////////////////////////////////////////////////////////////////////////////////////////

    function createTextureFeatureString(theTextureCount) {
        var myString = '[100[';
        for (var i = 0; i < theTextureCount; ++i) {
            myString += "paint";
            if (i != theTextureCount - 1) {
                myString += ",";
            }
        }
        myString += ']]';
        return myString;
    }
    function createTexcoordFeatureString(theTextureCount) {
        var myString = '[100[';
        for (var i = 0; i < theTextureCount; ++i) {
            myString += "uv_map";
            if (i != theTextureCount - 1) {
                myString += ",";
            }
        }
        myString += ']]';
        return myString;
    }

    function addTextureRequirements(theTextureCount) {
        var myRequiresNode = getDescendantByTagName(Public.material, "requires", false);
        if (myRequiresNode) {
            var myTextureFeatures = getDescendantByAttribute(myRequiresNode, "class", "textures", false);
            if (myTextureFeatures) {
                myTextureFeatures.values = createTextureFeatureString(theTextureCount);
            } else {
                myTextureFeatures = new Node('<feature class="textures" values="[100[paint]]"/>').firstChild;
                myRequiresNode.appendChild(myTextureFeatures);
            }

            var myTexcoordFeatures = getDescendantByAttribute(myRequiresNode, "class", "texcoord", false);
            if (myTexcoordFeatures) {
                myTexcoordFeatures.values = createTexcoordFeatureString(theTextureCount);
            } else {
                myTexcoordFeatures = new Node('<feature class="texcoord" values="[100[uv_map]]"/>').firstChild;
                myRequiresNode.appendChild(myTexcoordFeatures);
            }
        } else {
            throw new Exception("TextureOverlay material has no requires node.", fileline());
        }
    }

    var _myTextures        = null;
    var _myTexture         = null;
}

// Pure virtual base class
function ImageOverlayBase(Public, Protected, theManager, theSource, thePosition, theParent) {

    TextureOverlay(Public, Protected, theManager, thePosition, theParent);

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Private
    ///////////////////////////////////////////////////////////////////////////////////////////

    function addImage(theSource) {
        Protected.myImage = Node.createElement("image");
        theManager.images.appendChild(Protected.myImage);
        Protected.myImage.src  = theSource;
        Protected.myImage.name = theSource;
        Protected.myImage.resize = "pad";
        Protected.addTexture(Protected.myImage.id);
    }

    function setup() {
        if (typeof(theSource) == "string") { // detect strings
            addImage(theSource);
            theManager.scene.update(Renderer.IMAGES);
        } else if ("previousSibling" in theSource) { // detect nodes
            Protected.myImage = theSource;
            Protected.addTexture(Protected.myImage.id);
        } else if ("splice" in theSource) { // detect arrays
            for (var i = 0; i < theSource.length; ++i) {
                addImage(theSource[i]);
            }
            theManager.scene.update(Renderer.IMAGES);
        } else {
            print("### ERROR: Invalid type of source argument in addImage: " + typeof(theSource) + " " + fileline());
            return;
        }

        var mySize    = getImageSize(Protected.myImage);
        Public.width  = mySize.x;
        Public.height = mySize.y;
    }

    setup();
}

/// Creates an overlay and with an image as content
//  @param theManager   Object       The overlay manager for the used viewport
//  @param theSource    String/Node  Image file name or image node
//  @param thePosition  Vector2f     Pixelposition of the overlay (optional, default is [0,0])
//  @param theParent    Node         Parent overlay node (optional, default is toplevel)
function ImageOverlay(theManager, theSource, thePosition, theParent) {
    var Public    = this;
    var Protected = {};
    ImageOverlayBase(Public, Protected, theManager, theSource, thePosition, theParent);
}

/// Creates an overlay and with a movie as content
//  @param theManager    Object       The overlay manager for the used viewport
//  @param theSource     String/Node  Movie file name or movie node
//  @param thePosition   Vector2f     Pixelposition of the overlay (optional, default is [0,0])
//  @param theParent     Node         Parent overlay node (optional, default is toplevel)
//  @param theAudioFlag  Boolean      Play audio with the movie (optional, default is true)
function MovieOverlay(theManager, theSource, thePosition, theParent, theAudioFlag) {
    var Public    = this;
    var Protected = {};
    MovieOverlayBase(Public, Protected, theManager, theSource, thePosition, theParent, theAudioFlag);
}

// pure virtual base class
function MovieOverlayBase(Public, Protected, theManager, theSource, thePosition, theParent, theAudioFlag) {

    TextureOverlay(Public, Protected, theManager, thePosition, theParent);

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Public
    ///////////////////////////////////////////////////////////////////////////////////////////

    Public.movie getter = function() {
        return Public.image;
    }
    Public.movie setter = function(theMovie) {
        Public.image = theMovie;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Private
    ///////////////////////////////////////////////////////////////////////////////////////////

    function setup() {
        if (typeof(theSource) == "object") {
            Protected.myImage = theSource;
        } else {
            Protected.myImage = Node.createElement("movie");
            theManager.images.appendChild(Protected.myImage);
            Protected.myImage.src  = theSource;
            Protected.myImage.name = theSource;
            Protected.myImage.resize = "pad";
            if (theAudioFlag == undefined) {
                theAudioFlag = true;
            }
            Protected.myImage.audio = theAudioFlag;
            window.loadMovieFrame(Protected.myImage);
        }

        var mySize = null;
        if (Protected.myImage.src.search(/\.m60/) != -1) {
            mySize = getImageSize(Protected.myImage);
        } else {
            mySize = new Vector2i(Protected.myImage.width, Protected.myImage.height);
        }

        Public.width  = mySize.x;
        Public.height = mySize.y;
        Protected.addTexture(Protected.myImage.id);
    }

    setup();
}
