//=============================================================================
// Copyright (C) 2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

function ASSManager(theViewer) {
    this.Constructor(this, theViewer);
}

ASSManager.driver = null;

const VERBOSE_EVENTS = false;

ASSManager.prototype.Constructor = function(self, theViewer) {
    function setup() {
        if ( ! ASSManager.driver ) {
            ASSManager.driver = plug("ASSEventSource");
        }
        _myDriver = ASSManager.driver;

        _myWindow.addExtension( ASSManager.driver );
        _myViewer.registerSettingsListener( _myDriver, "ASSDriver" );
        _myViewer.registerSettingsListener( self, "ASSDriver" );

    }

    self.onUpdateSettings = function( theSettings ) {

        _mySettings = theSettings;

        if (_myValueOverlay) {
            var myMaterialId = getMaterialIdForValueDisplay();
            if (myMaterialId) {
                _myValueOverlay.material = _myScene.world.getElementById( myMaterialId );
            }

            _myValueOverlay.visible = new Number( _mySettings.childNode("ShowASSData").childNode("#text") );
            _myValueOverlay.position = _mySettings.childNode("SensorPosition").childNode("#text");
            _myValueOverlay.rotation = new Number( _mySettings.childNode("SensorOrientation").childNode("#text") );
            var myOldScale = new Vector2f( _myValueOverlay.scale );
            _myValueOverlay.scale =  _mySettings.childNode("SensorScale").childNode("#text"); 
            var myMirrorXFlag = new Number( _mySettings.childNode("MirrorX").childNode("#text"));
            var mySize = product( myOldScale, _myGridSize);
            if (myMirrorXFlag != 0) {
                _myValueOverlay.scale.x *= -1;
            }
            var myMirrorYFlag = new Number(_mySettings.childNode("MirrorY").childNode("#text"));
            if (myMirrorYFlag != 0) {
                _myValueOverlay.scale.y *= -1;
            }

            if ( _myInitialSettingsLoaded) {
                if ((myOldScale.x > 0 && _myValueOverlay.scale.x < 0) ||
                        (myOldScale.x < 0 && _myValueOverlay.scale.x > 0))
                {
                    _myValueOverlay.position.x += mySize.x;
                    _mySettings.childNode("SensorPosition").childNode("#text").nodeValue =
                            _myValueOverlay.position;
                }

                if ((myOldScale.y > 0 && _myValueOverlay.scale.y < 0) ||
                        (myOldScale.y < 0 && _myValueOverlay.scale.y > 0))
                {
                    _myValueOverlay.position.y += mySize.y;
                    _mySettings.childNode("SensorPosition").childNode("#text").nodeValue =
                            _myValueOverlay.position;
                }
            }
            _myInitialSettingsLoaded = true;
        }
    }

    self.onASSEvent = function( theEventNode ) {
        if (theEventNode.type == "configure") {
            if ( VERBOSE_EVENTS ) {
                print("ASSManager::onASSEvent: configure");
            }
            onConfigure( theEventNode );
        } else if (theEventNode.type == "add") {
            if ( VERBOSE_EVENTS ) {
                print("ASSManager::onASSEvent: add");
            }
        } else if (theEventNode.type == "move") {
            if ( VERBOSE_EVENTS ) {
                print("ASSManager::onASSEvent: move");
            }
        } else if ( theEventNode.type == "touch") {
            if ( VERBOSE_EVENTS ) {
                print("ASSManager::onASSEvent: touch at " + theEventNode.raw_position);
            }
        }
        if (theEventNode.type == "remove") {
            if ( VERBOSE_EVENTS ) {
                print("ASSManager::onASSEvent: remove");
            }
        }
    }

    self.driver getter = function() {
        return _myDriver;
    }

    function setupValueMaterials() {
        var myRasterNames = _myDriver.rasterNames;
        for (var i = 0; i < myRasterNames.length; ++i) {

            var myMaterial = _myScene.world.getElementById( myRasterNames[i] + "Material" );
            var myRaster = _myScene.world.getElementById( myRasterNames[i] );

            if ( ! myMaterial ) {
                myMaterial = Modelling.createUnlitTexturedMaterial(_myScene, myRaster );
                myMaterial.name = myRasterNames[i] + "Material";
                myMaterial.id = myRasterNames[i] + "Material";
                myMaterial.transparent = true;
            }

            var myXScale = _myGridSize.x / myRaster.width;
            var myYScale = _myGridSize.y / myRaster.height;
            myRaster.matrix.makeScaling( new Vector3f( myXScale, myYScale, 1));
            var myColorScale = 255 / 254;
            myRaster.color_scale = new Vector4f(myColorScale, myColorScale, myColorScale, 1);
            //myRaster.color_bias = [1, 1, 1, 1];
            myRaster.min_filter = TextureSampleFilter.nearest;
            myRaster.mag_filter = TextureSampleFilter.nearest;
            myRaster.texturepixelformat = "INTENSITY";

        }
    }

    function onConfigure( theEvent ) {

        _myGridSize = new Vector2f( theEvent.grid_size );

        setupValueMaterials();

        var myMaterial = _myScene.world.getElementById( getMaterialIdForValueDisplay() );
        _myValueOverlay = new ImageOverlay( _myScene, 
                _myScene.world.getElementById( myMaterial.childNode("textures").firstChild.image ));
        _myDriver.overlay = _myValueOverlay.node;

        if (_mySettings) {
            self.onUpdateSettings( _mySettings );
        }
    }

    function getMaterialIdForValueDisplay() {
        var myRasterNames = _myDriver.rasterNames;
        if ( myRasterNames.length == 0) {
            Logger.error("No value rasters.");
        }
        var myIndex = 0;
        if ( _mySettings ) {
            myIndex = new Number( 
                    _mySettings.childNode("CurrentValueDisplay", 0).childNode("#text"));
        }
        if (myIndex < 0 || myIndex >= myRasterNames.length) {
            Logger.warning("CurrentValueDisplay is out of bounds. Using zero.");
            myIndex = 0;
        }
        return myRasterNames[ myIndex ] + "Material";
    }

    self.valueColor getter = function() {
        var myMaterial = _myScene.world.getElementById( _myDriver.rasterNames[0] + "Material");
        if (myMaterial) {
            return myMaterial.properties.surfacecolor;
        }
        Logger.warning("Can not get valueColor. Materials have not been created yet.");
        return Vector4f(1, 1, 1, 1);
    }
    self.valueColor setter = function(theColor) {
        var myRasterNames = _myDriver.rasterNames;
        for (var i = 0; i < myRasterNames.length; ++i) {
            var myId = _myDriver.rasterNames[0] + "Material";
            var myMaterial = _myScene.world.getElementById( myId );
            if (myMaterial) {
                myMaterial.properties.surfacecolor = theColor;
            } else {
                Logger.warning("Can not set valueColor. Material with id '" + myId + "' has not been created yet.");
            }
        }
    }
    self.gridColor getter = function() {
        return _myDriver.gridColor;
    }
    self.gridColor setter = function(theColor) {
        _myDriver.gridColor = theColor;
    }

    self.cursorColor getter = function() {
        return _myDriver.cursorColor;
    }
    self.cursorColor setter = function(theColor) {
        _myDriver.cursorColor = theColor;
    }

    self.touchColor getter = function() {
        return _myDriver.touchColor;
    }
    self.touchColor setter = function(theColor) {
        _myDriver.touchColor = theColor;
    }

    self.textColor getter = function() {
        return _myDriver.textColor;
    }
    self.textColor setter = function(theColor) {
        _myDriver.textColor = theColor;
    }

    self.probeColor getter = function() {
        return _myDriver.probeColor;
    }
    self.probeColor setter = function(theColor) {
        _myDriver.probeColor = theColor;
    }



    var _myViewer = theViewer;
    var _myWindow = theViewer.getRenderWindow();
    var _myScene = theViewer.getScene();
    var _myDriver = null;
    var _mySettings = null;

    var _myGridSize =  new Vector2f(1, 1);
    var _myInitialSettingsLoaded = false;

    var _myValueOverlay = null;

    setup();
}


