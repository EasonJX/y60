//=============================================================================
// Copyright (C) 2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
// i know this must hurt, but i need them (vs)
use("Button.js")
use("Overlay.js")
function ASSManager(theViewer, theOSDFlag) {
    this.Constructor(this, theViewer, theOSDFlag);
}

ASSManager.driver = null;

const VERBOSE_EVENTS = false;
var QUIT_OSD = true;
var OSD_FILE = undefined;
const ENABLE_QUIT_OSD_TIME = 4.0;
const ENABLE_QUIT_OSD_DISTANCE = 50;//65.0;

ASSManager.prototype.Constructor = function(self, theViewer, theOSDFlag) {
    function setup() {
        if ( ! ASSManager.driver ) {
            ASSManager.driver = plug("ASSEventSource");
        }
        _myDriver = ASSManager.driver;

        _myWindow.addExtension( ASSManager.driver );
        _myViewer.registerSettingsListener( _myDriver, "ASSDriver" );
        _myViewer.registerSettingsListener( self, "ASSDriver" );


        if (theOSDFlag == undefined) {
            _hasQuitOSD = QUIT_OSD;
        } else {
            _hasQuitOSD = theOSDFlag;
        }
        print("======= has quit osd: " + _hasQuitOSD );

        if (_hasQuitOSD) {
            buildQuitOSD();
        }
        

    }

    self.onUpdateSettings = function( theSettings ) {

        _mySettings = theSettings;

        if (_myValueOverlay) {

            // XXX: shearing hack
            var myShearX = _mySettings.childNode("ShearX");
            if(myShearX) {
                _myDriver.shearX = Number(myShearX.childNode("#text"));
            }
            var myShearY = _mySettings.childNode("ShearY");
            if(myShearY) {
                _myDriver.shearY = Number(myShearY.childNode("#text"));
            }

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

    self.setOSDInvisible = function() {
        _myQuitOSD.visible = false;
        _myQuitCursorEvent = null;         
    }
 

    var _myEventQueues = {};
    var _myEventIDMQBs = {};
    self.onASSEvent = function( theEventNode ) {
        if (!(theEventNode.id in _myEventQueues)) {
            _myEventQueues[theEventNode.id] = new Array();
        }
        _myEventQueues[theEventNode.id].push(theEventNode);
        if (_myEventQueues[theEventNode.id].length > 1000) {
            _myEventQueues[theEventNode.id].shift();
        }
//print(theEventNode);
        if (theEventNode.type == "configure") {
            if ( VERBOSE_EVENTS ) {
                print("ASSManager::onASSEvent: configure");
            }
            onConfigure( theEventNode );
        } else if (theEventNode.type == "add") {
            if ( VERBOSE_EVENTS ) {
                print("ASSManager::onASSEvent: add");
            }
            if (_hasQuitOSD) {                    
                if (!_myQuitCursorEvent) {
                    _myQuitCursorEvent = theEventNode;
                    //print("#### added quit cursor");
                }
            }
            //_myEventIDMQBs[theEventNode.id] = new MaterialQuadBody();

        } else if (theEventNode.type == "move") {
            if ( VERBOSE_EVENTS ) {
                print("ASSManager::onASSEvent: move");
            }
            if (_hasQuitOSD) {                    
                if (_myQuitCursorEvent && _myQuitCursorEvent.id == theEventNode.id) {
                    var myTouchDuration = theEventNode.simulation_time - _myQuitCursorEvent.simulation_time;
                    var myHandTraveled = distance(theEventNode.position3D, _myQuitCursorEvent.position3D);
                    if (myHandTraveled > ENABLE_QUIT_OSD_DISTANCE) {
                        _myQuitCursorEvent = null;
                    } else if (myTouchDuration > ENABLE_QUIT_OSD_TIME) {
                        if(_myQuitOSD.visible == false) {
                            _myQuitOSD.moveToTop();
                            _myQuitOSD.visible = true;
                            window.setTimeout("setOSDInvisible", 5.0*1000, self);     
                        }    
                    }
                }
            }
            //_myEventMQBs[theEventNode.id].body.position = theEventNode.position3D;

        } else if ( theEventNode.type == "touch") {
            if ( VERBOSE_EVENTS ) {
                print("ASSManager::onASSEvent: touch at " + theEventNode.raw_position);
            }
            if (_hasQuitOSD) {                    
                // handle application quit
                 var myPosition = theEventNode.position3D;                    
                _myQuitCancelButton.onMouseButton(MOUSE_DOWN, myPosition.x, myPosition.y, 30);
                _myQuitCancelButton.onMouseButton(MOUSE_UP, myPosition.x, myPosition.y);
                _myQuitConfirmButton.onMouseButton(MOUSE_DOWN, myPosition.x, myPosition.y, 30);
                _myQuitConfirmButton.onMouseButton(MOUSE_UP, myPosition.x, myPosition.y);
            }
        }
        if (theEventNode.type == "remove") {
            if ( VERBOSE_EVENTS ) {
                print("ASSManager::onASSEvent: remove");
            }
            if (_hasQuitOSD) {                    
                if (_myQuitCursorEvent && _myQuitCursorEvent.id == theEventNode.id) {
                    _myQuitCursorEvent = null;
                }
            }
        }
    }

    self.getPrecedingEvent = function(theEvent, theNumber) {
        if (theEvent.id in _myEventQueues) {
            var thisQueue = _myEventQueues[theEvent.id];
            if (thisQueue.length > theNumber) {
                return thisQueue[thisQueue.length - theNumber - 1];
            }
            return thisQueue[0];
        } else {
            print("Sorry, no event in queue " + theEvent.id);
        }
    }

    self.driver getter = function() {
        return _myDriver;
    }
  function buildQuitOSD() {
        const myStyle = {
            color:             asColor("FFFFFF"),
            selectedColor:     asColor("FFFFFF"),
            textColor:         asColor("00FFFF"),
            font:              "${PRO}/testmodels/fonts/arial.ttf", 
            HTextAlign:        Renderer.LEFT_ALIGNMENT,
            fontsize:          18
        }
        
        var myOSDSize = new Vector2i(300, 80);
        var myImage = theViewer.getImageManager().getImageNode("OSD_Overlay");
        if (OSD_FILE != undefined) {
            myImage.src = OSD_FILE;
        } else {
            myImage.src = "shadertex/on_screen_display.rgb";
        }
        myImage.resize = "pad";

        _myQuitOSD = new ImageOverlay(theViewer.getScene(), myImage);
        var myImageSize = getImageSize(_myQuitOSD.image);
       _myQuitOSD.visible = false;

        var myColor = 0.3
        var myQuitText="";
        var myContinueText="";
        if (OSD_FILE == undefined) {
            _myQuitOSD.color = new Vector4f(myColor,myColor,myColor,0.75);
            myQuitText = "Quit";
            myContinueText="Continue";
            _myQuitOSD.width  = myOSDSize.x;
            _myQuitOSD.height = myOSDSize.y;
        } else {
            _myQuitOSD.width  = myImageSize.x;
            _myQuitOSD.height = myImageSize.y;
         }
       _myQuitOSD.position = new Vector2f((window.width - _myQuitOSD.width) / 2,
                                             (window.height - _myQuitOSD.height) / 2);
         
        var myButtonSize = new Vector2i((_myQuitOSD.width/2)-10,(_myQuitOSD.height/2)-10);
        var myButtonPos = new Vector2f(10,10);
        _myQuitConfirmButton = new TextButton(window.scene, "Confirm_Quit", myQuitText, myButtonSize, myButtonPos, myStyle, _myQuitOSD);
        _myQuitConfirmButton.onClick = function() {
            exit();
        }
        myStyle.HTextAlign = Renderer.RIGHT_ALIGNMENT,
        myButtonPos.x += myButtonSize.x;
        _myQuitCancelButton = new TextButton(window.scene, "Cancel_Quit", myContinueText, myButtonSize, myButtonPos, myStyle, _myQuitOSD);
        _myQuitCancelButton.onClick = function() {
            _myQuitOSD.visible = false;
        }
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
        if ( ! _myValueOverlay) {
            _myValueOverlay = new ImageOverlay( _myScene, 
                    _myScene.world.getElementById( myMaterial.childNode("textures").firstChild.image ));
        }
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
    var _myQuitCancelButton  = null;
    var _myQuitConfirmButton = null;
    var _myQuitOSD = null;
    var _myQuitCursorEvent = null;
    var _hasQuitOSD = false;
 setup();
}


