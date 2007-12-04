//=============================================================================
// Copyright (C) 2003-2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

// use this idiom in each level of inheritance and
// you'll know if you are the outermost .js file.
if (__main__ == undefined) var __main__ = "SceneViewer";

use("Y60JSSL.js");
use("Exception.js");
use("LightManager.js");
use("picking_functions.js");
use("HeartbeatThrober.js");
use("Playlist.js");
use("Glow.js");
use("SwitchNodeHandler.js");
use("AutoClicker.js");
//use("OcclusionUtils.js"); // XXX disabled, no longer needed

function BaseViewer(theArguments) {
    this.Constructor(this, theArguments);
}

BaseViewer.prototype.Constructor = function(self, theArguments) {
    self.getReleaseMode = function() {
        return _myReleaseMode;
    }

    self.glow getter = function() {
        return _myGlow ? _myGlow.getEnabled() : false; 
    }
    self.glow setter = function(theMode) {
        if (theMode) {
            if (_myGlow === null) {
                _myGlow = new Glow(self, 5, 2.5);
            }
            self.onRender = _myGlow.onRender;
        } else {
            delete self.onRender;
        }
        _activeViewport.glow = theMode;
        _myGlow.setEnabled(theMode);
    }
    self.getProfileMode = function() {
        return (_myProfileNode != null);
    }

    self.getShaderLibrary = function() {
        return _myShaderLibrary;
    }
    self.autoClicker getter = function() {
        if (!_myAutoClicker) {
            _myAutoClicker = new AutoClicker(self);
        }
        return _myAutoClicker;
    }
    
    self.setModelName = function(theModelName) {
        _myModelName = theModelName;
    }
    
    self.getModelName = function() {
        return _myModelName;
    }

    // [CH]: deprecated, use window.scene.world (etc.) instead.
    // Use these functions for fast access to the scene graph
    self.getWorld = function() {
        Logger.warning("getWorld() is deprecated, use window.scene.world instead");
        return _myWorld;
    }
    self.getMaterials = function() {
        Logger.warning("getMaterials() is deprecated, use window.scene.materials instead");
        return _myMaterials;
    }
    self.getLightSources = function() {
        Logger.warning("getLightSources() is deprecated, use window.scene.lightsources instead");
        return _myLightSources;
    }
    self.getShapes = function() {
        Logger.warning("getShapes() is deprecated, use window.scene.shapes instead");
        return _myShapes;
    }
    self.getAnimations = function() {
        Logger.warning("getAnimations() is deprecated, use window.scene.animations instead");
        return _myAnimations;
    }
    self.getCharacters = function() {
        Logger.warning("getCharacters() is deprecated, use window.scene.characters instead");
        return _myCharacters;
    }
    self.getImages = function() {
        Logger.warning("getImages() is deprecated, use window.scene.images instead");
        return _myImages;
    }

    self.getLightManager = function() {
        return _myLightManager;
    }

    self.getRenderWindow = function() {
        return _myRenderWindow;
    }

    self.setActiveViewport = function(theViewport) {
        Logger.info("Active viewport is name=" + theViewport.name + " id=" + theViewport.id);
        _activeViewport = theViewport;
    }
    self.getActiveViewport = function() {
        return _activeViewport;
    }
    
    self.getViewportAtWindowCoordinates = function(theX, theY) {
        var myLowestPos = new Vector2f(10000,10000);
        var myBestChoice = _activeViewport;
        for (var myViewPortIndex = 0 ; myViewPortIndex < window.scene.canvases.childNode(0).childNodesLength(); myViewPortIndex++) {
            var myViewPort = window.scene.canvases.childNode(0).childNode(myViewPortIndex);
            var myViewPortSize = new Vector2f(window.width * myViewPort.size[0], window.height * myViewPort.size[1]);
            var myViewPortPos = new Vector2f(window.width * myViewPort.position[0], window.height * myViewPort.position[1]);
            if ((theX >= myViewPortPos[0] && theX < myViewPortPos[0] + myViewPortSize[0] ) &&
                (theY >= myViewPortPos[1] && theY < myViewPortPos[1] + myViewPortSize[1] )) {
                    return myViewPort;
            }
            // if none matches, get the most left upper viewport
            if (myViewPortPos[0] <= myLowestPos.x && myViewPortPos[1] <= myLowestPos.y) {
                myBestChoice = myViewPort;
                myLowestPos.x = myViewPortPos[0];
                myLowestPos.y = myViewPortPos[1];
            }
        }        
        return myBestChoice;
    }
    
    self.getActiveCamera = function() {
        var myViewport = self.getActiveViewport();
        return myViewport.getElementById(myViewport.camera);
    }

    self.attachWindow = function(theRenderWindow) {
        _myRenderWindow = theRenderWindow;
        // register our event listener
        _myRenderWindow.eventListener = self;
        _myPicking = new Picking(_myRenderWindow);
    }

    self.setupWindow = function(theRenderWindow, theSetDefaultRenderingCap) {
        self.attachWindow(theRenderWindow);
        if (theSetDefaultRenderingCap == undefined || theSetDefaultRenderingCap) {
            _myRenderWindow.renderingCaps =
                Renderer.TEXTURE_3D_SUPPORT |
                Renderer.MULTITEXTURE_SUPPORT |
                Renderer.TEXTURECOMPRESSION_SUPPORT |
                Renderer.CUBEMAP_SUPPORT;
        }
   }

   self.getScene = function() {
       return _myRenderWindow.scene;
   }

   self.setScene = function(theScene, theCanvas, theSwitchNodeFlag) {
       var myStatus = _myRenderWindow.setSceneAndCanvas(theScene, theCanvas);
       if (!myStatus) {
           throw new Exception("Could not load model", fileline());
       }
       if (!theScene) {
           self.prepareScene(null, null, theSwitchNodeFlag);
       } else {
           var myCanvas = theCanvas ? theCanvas : getDescendantByTagName(theScene.dom, 'canvas', true);
           self.prepareScene(theScene, myCanvas, theSwitchNodeFlag);
       }
   }

    self.setMover = function(theMoverFactory, theViewport) {
        if (!theMoverFactory || theMoverFactory == undefined) {
            theMoverFactory = MoverBase;
        }
        var myNewMover = new theMoverFactory(theViewport);
        var myViewportId = getViewportId(theViewport);
        _myViewportMovers[myViewportId] = myNewMover;
        myNewMover.setMoverObject(myNewMover.getViewportCamera());
        return myNewMover;
    }

    self.getMover = function(theViewport) {
        var myIndex = getViewportId(theViewport);
        if (myIndex in _myViewportMovers) {
            return _myViewportMovers[myIndex];
        } else {
            return null;
        }
    }

    self.registerMover = function(theMoverFactory) {
        // should check if object type is already in list
        _myMoverFactories.push(theMoverFactory);
    }

    self.nextMover = function(theViewport) {
        var myViewportId = getViewportId(theViewport);

    	if (_myMoverFactories.length == 0) {
    	    return;
    	}

        // find next mover
        var myNextMoverIndex = 0;
        for (var i = 0; i < _myMoverFactories.length; ++i) {
            if (_myMoverFactories[i] == _myViewportMovers[myViewportId].constructor) {
                myNextMoverIndex = i+1;
                break;
            }
        }
        if (myNextMoverIndex >= _myMoverFactories.length) {
            myNextMoverIndex = 0;
        }

        // switch mover
        var myNewMover = self.setMover(_myMoverFactories[myNextMoverIndex], theViewport);
        print("Activated Mover: " + myNewMover.name);
    }

    self.setAutoNearFarPlane = function(theFlag) {
        _myAutoNearFarFlag = theFlag;
    }

    self.setCanvas = function(theCanvasNode) {
        if (theCanvasNode != _myRenderWindow.canvas) {
            _myRenderWindow.canvas = theCanvasNode;
        }
        _myLightManager.setupDefaultLighting(theCanvasNode);
        for (var i=0; i < theCanvasNode.childNodesLength('viewport'); ++i) {
            var myViewport = theCanvasNode.childNode('viewport');
            _myLightManager.setupHeadlight(myViewport);
        }
        _activeViewport = getDescendantByTagName(_myRenderWindow.canvas, 'viewport');
    }

    self.setCanvasByIndex = function(theIndex) {
        var myCanvasRoot = getDescendantByTagName(_myRenderWindow.scene.dom, "canvases", true);
        var myDefaultCamera = getDescendantByTagName(_myRenderWindow.scene.dom, "camera", true);
        if (myCanvasRoot.childNodes.length > theIndex) {
            self.setCanvas(myCanvasRoot.childNodes[theIndex]);
        } else {
            var myCanvas = Modelling.createCanvas(_myRenderWindow.scene, "AutoCanvas");
            var myViewport = myCanvas.childNode(0);
            myViewport.camera = myDefaultCamera.id;
            self.setCanvas(myCanvas);
        }
    }

    self.nextCamera = function(theViewport) {
        return self.setActiveCamera(_myActiveCameraIndex+1, theViewport);
    }
    self.prevCamera = function(theViewport) {
        return self.setActiveCamera(_myActiveCameraIndex-1, theViewport);
    }

    self.setActiveCamera = function(theCamera, theViewport) {
        if (!isNaN(theCamera)) {
            var myCameraIndex = theCamera;
            if (myCameraIndex < 0) {
                myCameraIndex = _myRenderWindow.scene.cameras.length-1;
            } else if (myCameraIndex > (_myRenderWindow.scene.cameras.length-1)) {
                myCameraIndex = 0;
            }
            try {
                theCamera = _myRenderWindow.scene.cameras[myCameraIndex];
                _myActiveCameraIndex = myCameraIndex;
            }
            catch (ex) {
                Logger.warning("No such camera index " + myCameraIndex);
                theCamera = _myRenderWindow.camera;
            }
        }
        if (!theViewport) {
            theViewport = self.getActiveViewport(); //getSingleViewport();
        }
        theViewport.camera = theCamera.id;

        if (self.getMover(theViewport)) {
            self.getMover(theViewport).setMoverObject(theCamera);
        }
        _myLightManager.setupHeadlight(theViewport);
        return theCamera;
    }

    self.addSkyBoxFromImage = function(theImageNode) {

        // material
        var myMaterialNode = _myRenderWindow.scene.world.getElementById(_mySkyboxMaterialId);
        if (!myMaterialNode) {
            myMaterialNode = Node.createElement("material");
            myMaterialNode.id = createUniqueId();
            myMaterialNode.name = "SkyboxMaterial";

            _mySkyboxMaterialId = myMaterialNode.id;
            _myRenderWindow.scene.materials.appendChild(myMaterialNode);
            _myRenderWindow.scene.world.skyboxmaterial = _mySkyboxMaterialId;

            // add texture units
            var myTextureUnitsNode = new Node("<textureunits><textureunit applymode='decal'/></textureunits>").firstChild;
            myMaterialNode.appendChild(myTextureUnitsNode);

            // add texture requirement
            var myTextureFeatures = new Node("<feature name='textures'>[100[skybox]]</feature>").firstChild;
            myMaterialNode.requires.appendChild(myTextureFeatures);
        }

        // texture unit
        var myTextureUnitNode = myMaterialNode.childNode("textureunits").firstChild;
 
        // texture
        var myTextureNode = myMaterialNode.getElementById(myTextureUnitNode.texture);
        if (!myTextureNode) {
            myTextureNode = Node.createElement("texture");
            myTextureNode.id = createUniqueId();
            myTextureNode.name = "SkyboxTexture";
            myTextureNode.wrapmode = "clamp_to_edge";
            myTextureNode.mipmap = false;

            _myRenderWindow.scene.textures.appendChild(myTextureNode);
            myTextureUnitNode.texture = myTextureNode.id;
        }

        myTextureNode.image = theImageNode.id;
    }

    self.addSkyBoxFromFile = function(theFileName, theTile) {

        if (theTile == undefined) {
            theTile = new Vector2i(1,6);
        }
        if (_mySkyboxMaterialId) {
            var mySkyboxMaterial = 
                _myRenderWindow.scene.world.getElementById(_mySkyboxMaterialId);
            var myTextureId = mySkyboxMaterial.childNode("textureunits").firstChild.texture;
            var myTexture = window.scene.textures.getElementById(myTextureId);
            var mySkyboxImage = mySkyboxMaterial.getElementById(myTexture.image);
            mySkyboxImage.src = theFileName;
            mySkyboxImage.tile = theTile;
            myTexture.wrapmode = "clamp_to_edge";
            _myRenderWindow.scene.world.skyboxmaterial = _mySkyboxMaterialId;
        } else {
            var myImageId = createUniqueId();
            var mySkyboxImage      = Node.createElement("image");
            mySkyboxImage.name     = theFileName;
            mySkyboxImage.id       = myImageId;
            mySkyboxImage.src      = theFileName;
            mySkyboxImage.tile     = theTile;
            mySkyboxImage.type     = "cubemap";

            _myRenderWindow.scene.images.appendChild(mySkyboxImage);

            var mySkyboxTexture = Modelling.createTexture(window.scene, mySkyboxImage);
            mySkyboxTexture.mipmap   = 0;
            mySkyboxTexture.wrapmode = "repeat";
            mySkyboxTexture.type     = "texture_cubemap";

            self.addSkyBoxFromImage(mySkyboxImage);
        }
    }

    self.removeSkyBox = function() {
        _myRenderWindow.scene.world.skyboxmaterial = "";
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    //  RenderWindow callback handlers
    //
    ///////////////////////////////////////////////////////////////////////////////////////////

    self.applyMaterialTable = function(theFilename) {

        // load-or-create material table
        if (!_myMaterialTable) {
            if (!theFilename) {
                theFilename = "materialtable.xml";
            }
            var myDoc = new Node();
            if (!fileExists(theFilename)) {
                Logger.info(theFilename+" does not exist. Could not apply MaterialTable");
                return new Node("<materialtable/>").firstChild; 
            }
            myDoc.parseFile(theFilename);
            if (myDoc.firstChild && myDoc.firstChild.nodeName == "materialtable") {
                _myMaterialTable = myDoc.firstChild;    
            } else {
                Logger.info("Could not apply MaterialTable: "+theFilename+" does not seem to contain a valid xml-structure.")
                return new Node("<materialtable/>").firstChild; 
            }
        }

        // merge materials
        for (var i=0; i<_myMaterialTable.childNodesLength(); ++i) {
            var myNode = _myMaterialTable.childNode(i);
            var myMaterial = getDescendantByName(_myMaterials, myNode.name, true);
            if (myMaterial) {
                Logger.info("Setting "+myMaterial.name+" to table value.");
                mergeMaterialProperties(myMaterial, myNode.childNode("properties"));
            } else {
                // in case of materialswitches
                var mySwitchNode = getDescendantByName(self.getScene().world, myNode.name, true);
                if (!mySwitchNode) {
                    continue;
                }
                if (mySwitchNode.nodeName == "body") {
                    Logger.info("Switchnode found in materialtable: "+mySwitchNode.name);
                    var myShape = mySwitchNode.getElementById(mySwitchNode.shape);
                    var myMaterialId = getDescendantByTagName(myShape, "primitives", true).firstChild.material; 
                    myMaterial = mySwitchNode.getElementById(myMaterialId);
                    mergeMaterialProperties(myMaterial, myNode.childNode("properties"));
                } else {
                    Logger.warning("Switchnode is not a body: "+mySwitchNode.name);
                }
            }
        }

        return _myMaterialTable;
    }

    function mergeMaterialProperties(theTargetMaterial, theTableNode) {

        //theTargetMaterial.removeChild(theTargetMaterial.childNode("properties"));
        //theTargetMaterial.appendChild(theTableNode.cloneNode(true));

        var myTargetProperties = theTargetMaterial.childNode("properties");
        for (var i = 0; i < theTableNode.childNodes.length; ++i) {
            var myNode = theTableNode.childNode(i);
            if (myNode.nodeName.search(/sampler/) != -1) {
                // skip samplers
                continue;
            }

            var myTargetProperty = getDescendantByName(myTargetProperties, myNode.name);
            if (myTargetProperty) {
                // remove property before replacing it
                myTargetProperties.removeChild(myTargetProperty);
            }
            myTargetProperties.appendChild(myNode.cloneNode(true));
        }
    }

    self.recollectSwitchNodes = function(theNode) {
        collectAllSwitchNodes(theNode);
    }

    self.onExit = function() {
        if (_myProfileNode) {
            _myProfileNode.name = self.getModelName();
            _myProfileNode.revision = revision();
            _myProfileNode.current = _myProfileNode.maxfps;

            // get or create profiles node
            var myProfilesNode = null;
            if (fileExists(_myProfileFilename)) {
                var myContent = readFileAsString(_myProfileFilename);
                if (myContent) {
                    myProfilesNode = new Node(myContent);
                    if (myProfilesNode) {
                        myProfilesNode = myProfilesNode.firstChild;
                    } else {
                        Logger.error("Unable to parse '" + _myProfileName + "'");
                    }
                } else {
                    Logger.error("Unable to open '" + _myProfileName + "'");
                }
            }
            if (!myProfilesNode) {
                myProfilesNode = new Node("<profiles/>").firstChild;
            }

            // find matching profile node
            var myNode = getDescendantByName(myProfilesNode, _myProfileNode.name);
            if (myNode) {
                _myProfileNode.previous = myNode.current;
                myProfilesNode.removeChild(myNode);
            }
            if (_myProfileNode.previous > 0.0) {
                _myProfileNode.gain = 1.0 - (_myProfileNode.previous / _myProfileNode.current);
            } else {
                _myProfileNode.gain = 0.0;
            }
            myProfilesNode.appendChild(_myProfileNode);

            // save
            writeStringToFile(_myProfileFilename, myProfilesNode);
        }
    }

    self.onPreViewport = function(theViewport) {
        if (theViewport.camera != "") {
            var myCamera = theViewport.getElementById(theViewport.camera);
            // calculate near/far planes from world size and distance camera-world
            var myWorldSize = _myRenderWindow.scene.getWorldSize(myCamera);
            if (self.getMover(theViewport)) {
                self.getMover(theViewport).setWorldSize(myWorldSize);
            }
            if (_myAutoNearFarFlag) {
                var myNearPlane = 0.1;
                var myFarPlane = 1000.0;
                if (myWorldSize > 0.0) {
                    myNearPlane = myWorldSize / 10000.0;
                    myFarPlane = myWorldSize * 2.0;
                }
                myCamera.frustum.near = myNearPlane;
                myCamera.frustum.far = myFarPlane;
            }
        }
        _myLightManager.onPreViewport(theViewport);
    }

    self.onPostViewport = function(theViewport) {
        _myLightManager.onPostViewport(theViewport);
    }

    self.onResize = function() {
    }

    self.drawCameraFrustums = function() {
        var myCameras = _myRenderWindow.scene.cameras;
        var myViewport = self.getActiveViewport();
        var myAspect = myViewport.width / myViewport.height;
        for (var i = 0; i < myCameras.length; ++i) {
            if (myCameras[i].id != myViewport.camera) {
                window.getRenderer().drawFrustum( myCameras[i], myAspect);
            }
        }
    }

    self.onPostRender = function() {
        if (_myDrawCameraFrustumFlag) {
            self.drawCameraFrustums();
        }
    }

    self.onFrame = function(theTime) {
        if (_myProfileNode) {
            var myFPS = _myRenderWindow.fps;
            if (myFPS > _myProfileNode.maxfps) {
                _myProfileNode.maxfps = myFPS;
                _myProfileNode.time = theTime;
                if (_myProfileNode.minfps <= 0.0) {
                    _myProfileNode.minfps = myFPS;
                }
            } else if (myFPS < _myProfileNode.minfps) {
                _myProfileNode.minfps = myFPS;
                _myProfileNode.time = theTime;
            }
        }
        if (_myHeartbeatThrober != null) {
            _myHeartbeatThrober.throb(theTime);
        }
        if (_myAutoClicker != null) {
            _myAutoClicker.onFrame();
        }
        
        _myLightManager.onFrame(theTime);
    }

    self.onMouseMotion = function(theX, theY) {
        var myViewportUnderMouse = _myPicking.getViewportAt(theX, theY);
        if (myViewportUnderMouse && myViewportUnderMouse != _myClickedViewport && self.getMover(myViewportUnderMouse)) {
            self.getMover(myViewportUnderMouse).onMouseMotion(theX, theY);
        }
        if (_myClickedViewport && self.getMover(_myClickedViewport)) {
            self.getMover(_myClickedViewport).onMouseMotion(theX, theY);
        }
    }

    self.onMouseButton = function(theButton, theState, theX, theY) {
        var myMover = null;
        if (theState) {
            var myViewport = _myPicking.getViewportAt(theX, theY);
            myMover = self.getMover(myViewport);
            _myClickedViewport = myViewport;
        } else {
            var myViewportUnderMouse = _myPicking.getViewportAt(theX, theY);
            if (myViewportUnderMouse) {
                if (_myClickedViewport) {
                    if (myViewportUnderMouse.id != _myClickedViewport.id) {
                        self.getMover(myViewportUnderMouse).onMouseButton(theButton, theState, theX, theY);
                    }
                    myMover = self.getMover(_myClickedViewport);
                    _myClickedViewport = null;
                } else {
                    myMover = self.getMover(myViewportUnderMouse);
                }
            } else {
                if (_myClickedViewport) {
                    myMover = self.getMover(_myClickedViewport);
                } else {
                    myMover = self.getMover();
                }
            }
        }
        if (myMover) {
            myMover.onMouseButton(theButton, theState, theX, theY);
        }
    }
    self.onMouseWheel = function(theDeltaX, theDeltaY) {
        var myMover = self.getMover(self.getActiveViewport());
        if (myMover) {
            myMover.onMouseWheel(theDeltaX, theDeltaY);
        }
    }

    self.enableHeartbeat = function(theFrequency, theHeartbeatfile) {
        if (_myHeartbeatThrober == null) {
            _myHeartbeatThrober = new HeartbeatThrober(false, 1, "${TEMP}/heartbeat.xml");
        }
        if(theHeartbeatfile != undefined && theFrequency != undefined) {
            _myHeartbeatThrober.use(true, theFrequency, theHeartbeatfile);
        } else {
            _myHeartbeatThrober.enable(true);
        }
    }

    self.enableNagios = function(thePort) {
        plug("NagiosPlugin");
        _myNagiosPlugin = new NagiosPlugin();
        _myNagiosPlugin.port = thePort;
        _myRenderWindow.addExtension(_myNagiosPlugin);
        return _myNagiosPlugin;
    }

    self.onAxis = function( theDevice, theAxis, theValue) {
        var myMover = self.getMover(self.getActiveViewport());
        if (myMover) {
            myMover.onAxis(theDevice, theAxis, theValue);
        }
    }

    self.onButton = function( theDevice, theButton, theState) {
        var myMover = self.getMover(self.getActiveViewport());
        if (myMover) {
            myMover.onButton(theDevice, theButton, theState);
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // private members
    //
    ///////////////////////////////////////////////////////////////////////////////////////////

    var _myRenderWindow          = null;
    var _myActiveCameraIndex     = 0;
    var _myModelName             = null;
    var _myShaderLibrary         = null;
    var _myReleaseMode           = true;
    var _myLightManager          = null;
    var _myGlow                  = null;

    const PROFILE_FILENAME = "profile.xml";
    var _myProfileFilename = null;
    var _myProfileNode     = null;

    // Camera movers
    var _myMoverFactories        = [];  // Array of mover constructors

    var _myViewportMovers        = [];
    var _myClickedViewport       = null;
    var _myAutoNearFarFlag       = true;
    var _activeViewport          = null;

    // For fast scene access
    var _myWorld                 = null;
    var _myMaterials             = null;
    var _myLightSources          = null;
    var _myShapes                = null;
    var _myCharacters            = null;
    var _myAnimations            = null;
    var _myImages                = null;

    var _mySkyboxMaterialId      = null;
    var _myHeartbeatThrober      = null;
    var _myNagiosPlugin          = null;
    var _myPicking               = null;
    var _myAutoClicker           = null;
    
    var _mySwitchNodes  = new Array();
    var _myMSwitchNodes = new Array();
    var _myTSwitchNodes = new Array();
    var _myMaterialTable = null;
    var _myDrawCameraFrustumFlag = false;

    self.__defineGetter__('_myPicking', function() { return _myPicking; });

    self.__defineGetter__('drawFrustums', function() { return _myDrawCameraFrustumFlag; });
    self.__defineSetter__('drawFrustums', function(theFlag) { _myDrawCameraFrustumFlag = theFlag; });


    function getSingleViewport() {
        if (_myRenderWindow.canvas.childNodesLength("viewport") == 1) {
            return _myRenderWindow.canvas.childNode("viewport");
        }
        Logger.warning("getSingleViewport called without a viewport in non-single viewport mode.");
        return null;
    }

    function getViewportId(theViewport) {
        if (theViewport) {
            return theViewport.id;
        }
        return self.getActiveViewport().id; //getSingleViewport().id;
    }

    function parseArguments(theArguments) {
        var myArgumentMap = [];
        if (theArguments) {
            for (var i = 0; i < theArguments.length; ++i) {
                var myArgument = theArguments[i];
                if (!_myShaderLibrary && myArgument.search(/shaderlib.*\.xml$/) != -1) {
                    // Take the first xml-file as shader library
                    _myShaderLibrary = myArgument;
                } else if (myArgument.search(/\.[xb]60$/) != -1 ||
                           myArgument.search(/\.st.$/) != -1 ||
                           myArgument.search(/\.x3d$/) != -1) {
                    _myModelName = myArgument;
                }

                myArgument = myArgument.split("=");
                if (myArgument.length > 1) {
                    myArgumentMap[myArgument[0]] = myArgument[1];
                } else {
                    myArgumentMap[myArgument[0]] = null;
                }
            }

            if ("rehearsal" in myArgumentMap) {
                _myReleaseMode = false;
            }
            if ("profile" in myArgumentMap) {
                _myProfileFilename = myArgumentMap["profile"];
                if (_myProfileFilename == null) {
                    _myProfileFilename = PROFILE_FILENAME;
                }
                _myProfileNode = new Node("<profile revision='0' name='' description='Frames-per-Second' current='0' previous='0' gain='0' time='0' maxfps='0' minfps='0'/>").firstChild;
                Logger.warning("Profiling enabled, filename=" + _myProfileFilename);
            }
        }
        return myArgumentMap;
    }

    // is called before first scene update
    self.preprocessScene = function(theScene) {
        // find all movienodes with no decoderhint and try to set it
        var myMovies = getDescendantsByTagName(theScene.images, "movie", false);

        if (myMovies) {
            for (var myMovieIndex = 0; myMovieIndex < myMovies.length; myMovieIndex++) {
                var myMovie = myMovies[myMovieIndex];
                if (myMovie.decoderhint == "") {
                    myMovie.decoderhint = new Playlist().getVideoDecoderHintFromURL(myMovie.src, false);
                    print("Set decoderhint for movie with source : " + myMovie.src + " to: " + myMovie.decoderhint);
                }
            }
        }

    }

    function collectSwitchNodes( theNode ) {
        for (var i = 0; i < theNode.childNodesLength(); ++i) {
            var myChild = theNode.childNode(i);
            if ( !("name" in myChild) ) {
                continue;
            }
            var myName = new String( myChild.name );
            if ( myName.match(/^switch_.*/)) {
                _mySwitchNodes.push( new SwitchNodeHandler( myChild ) );
            }
            if ( myName.match(/^mswitch_.*/)) {
                _myMSwitchNodes.push( new MSwitchNodeHandler( myChild ) );
            }
            if ( myName.match(/^tswitch_.*/) && myChild.nodeName == "material") {
                _myTSwitchNodes.push( new TSwitchNodeHandler( theNode.childNode(i)) );
            }
            collectSwitchNodes( myChild );
        }
    }

    function collectAllSwitchNodes(theNode) {
        _mySwitchNodes  = new Array();
        _myMSwitchNodes = new Array();
        _myTSwitchNodes = new Array();
    
        if (theNode == null) {
            collectSwitchNodes(window.scene.world);
        } else {
            collectSwitchNodes(theNode);
        }
        collectSwitchNodes(window.scene.materials);
    }

    self.getSwitchNodes = function() {
        return _mySwitchNodes;
    }

    self.getMaterialSwitchNodes = function() {
        return _myMSwitchNodes;
    }

    self.getTextureSwitchNodes = function() {
        return _myTSwitchNodes;
    }

    self.prepareScene = function (theScene, theCanvas, theSwitchNodeFlag) {
        if (theScene) {
            // Cache main scene nodes for fast access
            var myWorlds    = getDescendantByTagName(theScene.dom, "worlds", false);
            
            _myWorld        = theScene.world;
            _myMaterials    = theScene.materials;
            _myLightSources = theScene.lightsources;
            _myAnimations   = theScene.animations;
            _myCharacters   = theScene.characters;
            _myShapes       = theScene.shapes;
            _myImages       = theScene.images;

            if (!_myWorld || !_myMaterials || !_myLightSources || !_myCharacters || !_myAnimations || !_myShapes) {
                throw new Exception("Could not find world, materials, lightsources or shapes node", fileline());
            }
            _myLightManager = new LightManager(theScene, _myWorld);
            if (theCanvas) {
                self.setCanvas(theCanvas);
            } else {
                self.setCanvas(_myRenderWindow.canvas);
            }

            if (theSwitchNodeFlag == undefined || theSwitchNodeFlag) {
                collectAllSwitchNodes();
                //setupOcclusionMaterials(theScene); // XXX disabled, no longer needed
            }

        } else {
            Logger.trace("prepareScene has no scene");

            // No scene
            _myWorld = null;
            _myMaterials = null;
            _myLightSources = null;
            _myAnimations = null;
            _myCharacters = null;
            _myShapes = null;
            _myImages = null;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////
    //
    // Constructor
    //
    ///////////////////////////////////////////////////////////////////////////////////////////

    self.arguments = parseArguments(theArguments);

    var myShaderLibrary = self.getShaderLibrary();
    if (myShaderLibrary) {
        GLResourceManager.prepareShaderLibrary(myShaderLibrary);
    } else {
        Logger.warning("No Shaderlibrary name given found, default library will be loaded");
    }
}
