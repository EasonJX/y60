//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
if (__main__ == undefined) var __main__ = "ASSDriverTest";

use("SceneViewer.js");

const DISPLAY_SCALE = 20;
const X_MIRROR = false;
const Y_MIRROR = true;
const ORIENTATION = 0.5 * Math.PI;

window = new RenderWindow();

function ASSDriverTestApp(theArguments) {
    this.Constructor(this, theArguments);
}

ASSDriverTestApp.prototype.Constructor = function(self, theArguments) {

    //////////////////////////////////////////////////////////////////////
    //
    // Constructor
    //
    //////////////////////////////////////////////////////////////////////

    SceneViewer.prototype.Constructor(self, theArguments);
    var Base = [];
    var _myMaterial = null;
    var _myMarkers = [];
    var _myGotDataFlag = false;
    var _myDriver = null;
    var _myCrosshairShape = null;
    var _myGroup = null;
    var _myDisplaySize3D = null;


    //////////////////////////////////////////////////////////////////////
    //
    // public members
    //
    //////////////////////////////////////////////////////////////////////

    // setup
    Base.setup = self.setup;
    self.setup = function(theWidth, theHeight, theTitle) {
        Base.setup(theWidth, theHeight, false, theTitle);
        window.resize(theWidth, theHeight);
        _myDriver = plug("ASSDriver");
        window.addExtension(_myDriver);
        //self.registerSettingsListener( _myDriver, "ASSDriver" );

        _myDriver.threshold = 80;

        print("setup done");
    }

    function createDisplay(theRaster) {

        var myGridSize = _myDriver.gridSize;
        _myDisplaySize3D = new Vector3f(myGridSize.x * DISPLAY_SCALE , myGridSize.y * DISPLAY_SCALE,0);

        _myGroup = Modelling.createTransform( window.scene.world );
        _myGroup.name = "SensorDisplay";

        if (X_MIRROR) {
            _myGroup.scale.x = -1;
        }
        if (Y_MIRROR) {
            _myGroup.scale.y = -1;
        }
        _myGroup.orientation.assignFromEuler( new Vector3f(0, 0, ORIENTATION));
        _myGroup.position = product( product( product(_myDisplaySize3D, _myGroup.scale),
                     _myGroup.orientation), -0.5);
        _myGroup.position.z = -400;

        _myMaterial = Modelling.createUnlitTexturedMaterial(window.scene, "testbild00.rgb");
        var myQuad = Modelling.createQuad(window.scene, _myMaterial.id, [0,0,0], _myDisplaySize3D);
        var myBody = Modelling.createBody(_myGroup, myQuad.id );

        var myBlueMaterial = Modelling.createUnlitTexturedMaterial(window.scene, "",
                            "Blue", true, false, 1, [0, 0, 1, 1]);        
        myBlueMaterial.properties.surfacecolor = [0, 0, 1, 1];
        var myFrameShape = Modelling.createQuad(window.scene, myBlueMaterial.id, [0,0,0], _myDisplaySize3D);
        myFrameShape.childNode("primitives", 0).childNode(0).type = "lineloop";
        myBody = Modelling.createBody(_myGroup, myFrameShape.id );
        myBody.position.z = 1;
        
        var myRedMaterial = Modelling.createUnlitTexturedMaterial(window.scene, "",
                            "Blue", true, false, 1, [1, 0, 0, 1]);        
        myRedMaterial.properties.surfacecolor = [1, 0, 0, 1];
        var myOriginMarkerShape = Modelling.createCrosshair(window.scene, myRedMaterial.id, 
                                                 1, 2, "Crosshair"); 
        var myOriginMarker = Modelling.createBody(_myGroup, myOriginMarkerShape.id );
        myOriginMarker.position.y = _myDisplaySize3D.y;
        myOriginMarker.position.z = 1;
        myOriginMarker.scale = new Vector3f(3, 3, 3);


        _myCrosshairShape = Modelling.createCrosshair(window.scene, myBlueMaterial.id, 
                                                 1, 2, "Crosshair"); 

        var myOldId = _myMaterial.childNode("textures", 0).childNode(0).image;
        var myOldImage = window.scene.dom.getElementById(myOldId);
        myOldImage.parentNode.removeChild( myOldImage );
        _myMaterial.childNode("textures", 0).childNode(0).image = theRaster.id;
        var myXScale = theRaster.width / nextPowerOfTwo( theRaster.width );
        var myYScale = theRaster.height / nextPowerOfTwo( theRaster.height );
        theRaster.matrix.makeScaling( new Vector3f( myXScale, myYScale, 1));
        var myColorScale = 255 / _myDriver.maxOccuringValue;
        theRaster.color_scale = new Vector4f(myColorScale, myColorScale, myColorScale, 1);
        theRaster.min_filter = TextureSampleFilter.nearest;
        theRaster.mag_filter = TextureSampleFilter.nearest;


    }

    Base.onFrame = self.onFrame;
    self.onFrame = function(theTime) {
        Base.onFrame(theTime);

        // hack!
        if ( ! _myGotDataFlag ) {
            var myRaster = window.scene.dom.getElementById("ASSRawRaster");
            //var myRaster = window.scene.dom.getElementById("ASSBinaryRaster");
            if (myRaster) {

                createDisplay(myRaster);

                _myGotDataFlag = true;
            }
        }

        var myPositions = _myDriver.positions;
        const myPixelCenterOffset = 10;

        while (_myMarkers.length < myPositions.length) {
            var myNewMarker = Modelling.createBody(_myGroup, _myCrosshairShape.id );
            myNewMarker.position.z = 1;
            myNewMarker.scale = new Vector3f(3, 3, 3);
            _myMarkers.push( myNewMarker );
            //print("spawn marker");
        }
        while (myPositions.length < _myMarkers.length) {
            var myOldMarker = _myMarkers.pop();
            _myGroup.removeChild( myOldMarker );
            //print("remove marker");
        }
        for (var i = 0; i < myPositions.length; ++i) {
            var myPos = myPositions[i];
            _myMarkers[i].position.x = DISPLAY_SCALE * myPos.x + myPixelCenterOffset;
            _myMarkers[i].position.y = _myDisplaySize3D.y - DISPLAY_SCALE * myPos.y - myPixelCenterOffset;
            //print ("in: " + myPos + " out: " + _myMarkers[i].position.xy);
        }
        /*
        if (_myMarkers.length) {
            print("=====");
        }
        */
    }

    Base.onKey = self.onKey;
    self.onKey = function(theKey, theKeyState, theX, theY, theShiftFlag, theCtrlFlag, theAltFlag) {
         if (theCtrlFlag) {
            Base.onKey(theKey, theKeyState, theX, theY, theShiftFlag, theCtrlFlag, theAltFlag);
             return;
         }
         if (!theKeyState) {
            return;
        }
    }

    Base.onMouseButton = self.onMouseButton;
    self.onMouseButton = function( theButton, theState, theX, theY ) {
        Base.onMouseButton( theButton, theState, theX, theY);
    }

}

if (__main__ == "ASSDriverTest") {
    print("ASSDriverTest");
    try {
        var ourASSDriverTestApp = new ASSDriverTestApp(
                [expandEnvironment("${PRO}") + "/src/Y60/shader/shaderlibrary_nocg.xml"]);
        ourASSDriverTestApp.setup(600, 600, "ASSDriverTest");
        ourASSDriverTestApp.go();
    } catch (ex) {
        print("-------------------------------------------------------------------------------");
        print("### Error: " + ex);
        print("-------------------------------------------------------------------------------");
        exit(1);
    }
}

