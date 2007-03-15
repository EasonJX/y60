//=============================================================================
// Copyright (C) 1993-2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
use("BaseViewer.js");
use("PreferenceDialog.js");
use("SceneViewerDialog.js");
use("DebugVisual.js");
use("ClassicTrackballMover.js");
use("FlyMover.js");
use("WalkMover.js");
use("GUIUtils.js");
use("SwitchNodeMenu.js");
use("MaterialEditorFunctions.js");

var ourHandler           = {};

use("StatusBar.js");
use("GtkAnimationManager.js");

const REVISION_STRING    = revision();

var window               = new RenderArea();

var ourGlade             = null;
var ourViewer            = null;
var ourMainWindow        = null;
var ourPreferenceDialog  = null;
var ourSceneViewerDialog = null;
var ourCameraPopup       = null;
var ourAnimationManager  = null;
var ourWindowState = {width: 800, height: 600, position: [0, 0]};
var ourMaterialTable     = null;

var ourCoordinateSystem  = null;
var ourStatusBar         = null;
var ourMaterialComboBox  = null;
var ourLastMaterial      = null;

var ourPatchObject			 = null;

var GLADE_FILE = "../GLADE/rendergirl.glade";

//=================================================
//
//  script command line parameters
//
//=================================================

var ourAllowedOptions = {
//    'unit-size': "%d"
};

//=================================================
//
//  Gtk Signal Handlers
//
//=================================================

ourHandler.on_mainWindow_realize = function() {
}

//   Signal Handlers for Preference Dialog
//
//=================================================
ourHandler.on_fog_disabled = function() {
  	ourGlade.get_widget("fog_start_box").hide();
  	ourGlade.get_widget("fog_end_box").hide();
  	ourGlade.get_widget("fog_density_box").hide();
  	ourGlade.get_widget("fog_color_box").hide();
  	ourGlade.get_widget("fog_type_box").hide();
}

ourHandler.on_fog_linear = function() {
  	ourGlade.get_widget("fog_start_box").show();
  	ourGlade.get_widget("fog_end_box").show();
  	ourGlade.get_widget("fog_density_box").hide();
  	ourGlade.get_widget("fog_color_box").show();
  	ourGlade.get_widget("fog_type_box").hide();
}

ourHandler.on_fog_exponential = function() {
  	ourGlade.get_widget("fog_start_box").hide();
  	ourGlade.get_widget("fog_end_box").hide();
  	ourGlade.get_widget("fog_density_box").show();
  	ourGlade.get_widget("fog_color_box").show();
  	ourGlade.get_widget("fog_type_box").show();
}

ourHandler.on_range_start = function() {
		ourGlade.get_widget("range_start_label").text=ourGlade.get_widget("range_start").value.toFixed(2);
}

ourHandler.on_range_end = function() {
		ourGlade.get_widget("range_end_label").text=ourGlade.get_widget("range_end").value.toFixed(2);
}

ourHandler.on_fog_density = function() {
		ourGlade.get_widget("fog_density_label").text=ourGlade.get_widget("fog_density").value.toFixed(2);
}

//=================================================
// File Menu Item Handlers
//=================================================

ourHandler.on_new_activate = function() {
    ourViewer.setModelName("");
    var myScene = new Scene();
    myScene.setup();
    ourViewer.setScene(myScene);
    var myCanvas = getDescendantByTagName(myScene.dom, 'canvas', true);
    ourViewer.setCanvas(myCanvas);
    ourStatusBar.set("New scene");
    setupGUI();
    ourSceneViewerDialog.setBaseNode(window.scene.world);
    window.queue_draw();
}

ourHandler.on_include_activate = function() {
    var isPaused = window.pause;
    window.pause = true;
    var myFilename = getFilenameDialog("Include Scene", FileChooserDialog.ACTION_OPEN);
    
    if (myFilename) {
        var myIncludeNode = new Node("<include/>").firstChild;
        myIncludeNode.src = myFilename;
        window.scene.world.appendChild(myIncludeNode);
        window.scene.update(Scene.WORLD);
        ourViewer.lastSwitched = {};
    }
    ourViewer.recollectSwitchNodes(); 
    ourViewer.setupSwitchNodeMenu();
    ourMaterialTable = ourViewer.applyMaterialTable();
		ourSceneViewerDialog.setBaseNode(window.scene.world);
    window.pause = false;
}

ourHandler.on_save_activate = function() {
    var myFilename = ourViewer.getModelName();
    var myBinaryFlag = (myFilename.search(/\.b60$/i) != -1);
    window.scene.save(myFilename, myBinaryFlag);
    ourStatusBar.set("Saved scene: " + myFilename);
}

ourHandler.on_save_as_activate = function(theArguments) {
    var myFilename = theArguments;
    if (typeof(myFilename) == "object") {
        myFilename = askUserForFilename("Save scene");
    }

    if (myFilename) {
        var myBinaryFlag = (myFilename.search(/\.b60$/i) != -1);
        window.scene.save(myFilename, myBinaryFlag);
        ourStatusBar.set("Saved scene as: " + myFilename);
    }
}

ourHandler.on_open_activate = function(theArguments) {
    var isPaused = window.pause;
    window.pause = true;
    var myFilename = theArguments;
    if (typeof(theArguments) == "object") {
        myFilename = getFilenameDialog("Open Scene", FileChooserDialog.ACTION_OPEN);
    }
    if (myFilename) {
        ourViewer.loadModel( myFilename );
    }
    ourSceneViewerDialog.setBaseNode(window.scene.world);
    window.pause = isPaused;
}



ourHandler.on_save_screenshot_activate = function(theArguments) {
    var myFilename = theArguments;
    if (typeof(myFilename) == "object") {
        myFilename = askUserForFilename("Save screenshot");
    }
    if (myFilename) {
        window.saveBuffer(myFilename);
        ourStatusBar.set("Saved screenshot: " + myFilename);
    }
}

//=================================================
// Edit Menu Item Handlers
//=================================================

ourHandler.on_preferences_activate = function() {
    ourPreferenceDialog.show();
}

//=================================================
// View Menu Item Handlers
//=================================================

ourHandler.on_scene_activate = function() {
	ourSceneViewerDialog.show();
}


ourHandler.on_normals_activate = function(theMenuItem) {
		window.drawnormals = theMenuItem.active;
    ourStatusBar.set("Normals " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}

ourHandler.on_wireframe_activate = function(theMenuItem) {
    window.wireframe = theMenuItem.active;
    ourStatusBar.set("Wireframe " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}

ourHandler.on_texturing_activate = function(theMenuItem) {
    window.texturing = theMenuItem.active;
    ourStatusBar.set("Texturing " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}

ourHandler.on_glow_activate = function(theMenuItem) {
    ourViewer.glow = theMenuItem.active;
    ourStatusBar.set("Glow " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}

ourHandler.on_flatshading_activate = function(theMenuItem) {
    window.flatshading = theMenuItem.active;
    ourStatusBar.set("Flatshading " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}

ourHandler.on_backface_culling_activate = function(theMenuItem) {
    window.backfaceCulling = theMenuItem.active;
    ourStatusBar.set("Backface culling " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}

ourHandler.on_coordinate_system_activate = function(theMenuItem) {
    if (!ourCoordinateSystem) {
        ourCoordinateSystem = new DebugVisual(ourViewer.getWorld(), ourViewer);
        ourCoordinateSystem.setup();
    }

    ourCoordinateSystem.showAllCoordinates(theMenuItem.active);
    ourStatusBar.set("Coordinate system " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}

ourHandler.on_fullscreen_activate = function(theMenuItem) {
    if (theMenuItem.active) {
        ourGlade.get_widget("main_toolbar").hide();
        ourGlade.get_widget("statusbar").hide();
        ourGlade.get_widget("menu_box").hide();
        ourGlade.get_widget("mainWindow").decorated = false;
        ourWindowState = { size: ourMainWindow.size, position: ourMainWindow.position};
        ourMainWindow.resize(ourMainWindow.screenSize.x, ourMainWindow.screenSize.y);
        ourMainWindow.position = [0, 0];
    } else {
        ourGlade.get_widget("main_toolbar").show();
        ourGlade.get_widget("statusbar").show();
        ourGlade.get_widget("menu_box").show();
        ourGlade.get_widget("mainWindow").decorated = true;

        // Well the window size we get from gtk is not the same we have to put into resize to get
        // the same result :-(
        ourMainWindow.size = [ourWindowState.size.x - 8, ourWindowState.size.y - 34];
        ourMainWindow.position = ourWindowState.position;
    }
}

//=================================================
// Camera Menu Item Handlers
//=================================================

ourHandler.on_previous_camera_activate = function() {
    var myCamera = ourViewer.prevCamera();
    ourStatusBar.set("Active camera: " + myCamera.name);
    window.queue_draw();
}

ourHandler.on_next_camera_activate = function() {
    var myCamera = ourViewer.nextCamera();
    ourStatusBar.set("Active camera: " + myCamera.name);
    window.queue_draw();
}

ourHandler.on_picking_trackball_activate = function(theMenuItem) {
    if (theMenuItem.active) {
        ourViewer.setMover(ClassicTrackballMover);
        ourViewer.getMover().setCentered(false);
        var myButton = ourGlade.get_widget("picking_trackball_button");
        if (!myButton.active) {
            myButton.active = true;
        }
        ourStatusBar.set("Activate picking trackball mover");
    }
}

ourHandler.on_centered_trackball_activate = function(theMenuItem) {
    if (theMenuItem.active) {
        ourViewer.setMover(ClassicTrackballMover);
        ourViewer.getMover().setCentered(true);
        var myButton = ourGlade.get_widget("centered_trackball_button");
        if (!myButton.active) {
            myButton.active = true;
        }
        ourStatusBar.set("Activate centerd trackball mover");
    }
}

ourHandler.on_fly_mover_activate = function(theMenuItem) {
    if (theMenuItem.active) {
        ourViewer.setMover(FlyMover);
        var myButton = ourGlade.get_widget("flymover_button");
        if (!myButton.active) {
            myButton.active = true;
        }
        ourStatusBar.set("Activate fly mover");
    }
}
ourHandler.on_walk_mover_activate = function(theMenuItem) {
    if (theMenuItem.active) {
        ourViewer.setMover(WalkMover);
        var myButton = ourGlade.get_widget("walkmover_button");
        if (!myButton.active) {
            myButton.active = true;
        }
        ourStatusBar.set("Activate walk mover");
    }
}

ourHandler.on_zoom_in_activate = function() {
    var myHfov = window.camera.frustum.hfov;
    if (myHfov > 5) {
        myHfov -= 5;
    } else if (myHfov > 0.1) {
        myHfov -= 0.1;
    }
    window.camera.frustum.hfov = myHfov;
    ourStatusBar.set("Zoom to " + getFocalLength(myHfov).toFixed(1) + "mm (HFOV: " + myHfov.toFixed(1) + ")");
    window.queue_draw();
}

ourHandler.on_zoom_out_activate = function() {
    var myHfov = window.camera.frustum.hfov;
    if (myHfov < 5) {
        myHfov += 0.1;
    } else if (myHfov < 175) {
        myHfov += 5;
    }
    window.camera.frustum.hfov = myHfov;
    ourStatusBar.set("Zoom to " + getFocalLength(myHfov).toFixed(1) + "mm (HFOV: " + myHfov.toFixed(1) + ")");
    window.queue_draw();
}

ourHandler.on_reset_camera_activate = function(theMenuItem) {
    ourViewer.getMover().reset();
    ourStatusBar.set("Reset mover");
    window.queue_draw();
}

//=================================================
// Light Menu Item Handlers
//=================================================

ourHandler.on_lighting_activate = function(theMenuItem) {
    window.lighting = theMenuItem.active;
    ourStatusBar.set("Lighting " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}

ourHandler.on_headlight_activate = function(theMenuItem) {
    ourViewer.getLightManager().enableHeadlight(theMenuItem.active);
    ourStatusBar.set("Headlight " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}

ourHandler.on_sunlight_activate = function(theMenuItem) {
    ourViewer.getLightManager().enableSunlight(theMenuItem.active);
    ourStatusBar.set("Sunlight " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}

ourHandler.on_quit_activate = function() {
    Logger.info("Quit");
    ourViewer.finalize();
    GtkMain.quit();
}

//=================================================
// Tools Menu Item Handlers
//=================================================
// Tools->Material Editor
//=================================================
ourHandler.on_material_dlg_save_clicked = function() {
    var isPaused = window.pause;
    window.pause = true;
    var myFilename = getFilenameDialog("Save Material", FileChooserDialog.ACTION_SAVE);
    
    var myMaterial = getSelectedMaterial();
    if (myMaterial && myFilename) {
        if (myFilename.indexOf("xml") == -1) {
            myFilename = myFilename+".xml";
        }
        
        var myNode = myMaterial.childNode("properties"); //.cloneNode(true);
        myNode.saveFile(myFilename, false);
    }

    window.pause = isPaused;
}

ourHandler.on_material_dlg_save_table_clicked = function() {
    var myMaterial = getSelectedMaterial();
    var myMaterialName = myMaterial.name;
    if ("mswitch_"+myMaterial.name in ourViewer.lastSwitched) {
        myMaterialName = ourViewer.lastSwitched["mswitch_"+myMaterial.name];
    }
    
    Logger.info("appending active properties for: "+myMaterial.name+" to materialtable as: "+myMaterialName);
    
    var myNode = getDescendantByName(ourMaterialTable, myMaterialName, true);
    if (myNode) {
        myNode.parentNode.removeChild(myNode);
    }

    var myNewNode = new Node("<material name=\""+myMaterialName+"\"/>").firstChild;
    myNewNode.appendChild(myMaterial.childNode("properties").cloneNode(true));
    
    ourMaterialTable.appendChild(myNewNode);
    ourMaterialTable.saveFile("materialtable.xml");
}

ourHandler.on_material_dlg_load_clicked = function() {
    var isPaused = window.pause;
    window.pause = true;
    var myFilename = getFilenameDialog("Open Material", FileChooserDialog.ACTION_OPEN);
     
    if (!myFilename) {
        window.pause = isPaused;
        ourStatusBar.set("Invalid or incomplete Filename");
        return;
    }    

    var myNode = Node.createDocument();
    myNode.parseFile(myFilename);
    if (!myNode) {
        ourStatusBar.set("File is no XML Node");
        return;
    }
    var myMaterial = getSelectedMaterial();
    myMaterial.replaceChild(myNode.firstChild, myMaterial.childNode("properties"));

    window.pause = isPaused;
}

ourHandler.on_material_dlg_close_clicked = function() {
    var myMaterialEditor = ourGlade.get_widget("dlgMaterialEditor");
    myMaterialEditor.hide();
}

ourHandler.on_material_editor_activate = function() {
    if (window.scene.materials.childNodesLength() == 0) {
        ourStatusBar.set("No Materials in Scene.");
        return;
    }
  
    ourStatusBar.set("Material Editor started.");
  
    var myMaterialEditor = ourGlade.get_widget("dlgMaterialEditor");

    ourMaterialComboBox = new ComboBoxText();
    ourGlade.get_widget("material_box").pack_end(ourMaterialComboBox, false, false);
   
    hideFloatSliders();

    ourMaterialComboBox.signal_changed.connect(ourMaterialComboBox, "on_changed");
    ourMaterialComboBox.on_changed = function() {
        for (i = 0; i < window.scene.materials.childNodesLength(); ++i) {
            if (window.scene.materials.childNode(i).name == this.active_text) {
                updateMaterialEditor();
                var myMaterial = window.scene.materials.childNode(i);
            }
        }
    }
    
    ourMaterialComboBox.show();

    // fill material choice rolldown with all available materials
    var myLabelsArray = new Array();
    var i=0;
    for (i=0; i<window.scene.materials.childNodesLength(); ++i) {
        myLabelsArray.push(window.scene.materials.childNode(i).name);
    }
    myLabelsArray.sort();

    for (i=0; i<myLabelsArray.length; ++i) {
        ourMaterialComboBox.append_text(myLabelsArray[i]);
    }

    // enable first element
    if (myLabelsArray.length > 0) {
        var myMaterial = (ourLastMaterial != null ? ourLastMaterial : myLabelsArray[0]);
        //print("Material:" + myMaterial);
        ourMaterialComboBox.active_text = myMaterial;
    }
  	
  	
    myMaterialEditor.show();
    
}

//=================================================
// Tools->Bounding boxes
//=================================================
ourHandler.on_bb_off_activate = function() {
    window.getRenderer().boundingVolumeMode = Renderer.BV_NONE;
    ourStatusBar.set("Bounding volume mode: off");
    window.queue_draw();
}
ourHandler.on_bb_body_activate = function() {
    window.getRenderer().boundingVolumeMode = Renderer.BV_BODY;
    ourStatusBar.set("Bounding volume mode: body");
    window.queue_draw();
}
ourHandler.on_bb_shape_activate = function() {
    window.getRenderer().boundingVolumeMode = Renderer.BV_SHAPE;
    ourStatusBar.set("Bounding volume mode: shape");
    window.queue_draw();
}
ourHandler.on_hierarchy_activate = function() {
    window.getRenderer().boundingVolumeMode = Renderer.BV_HIERARCHY;
    ourStatusBar.set("Bounding volume mode: hierarchy");
    window.queue_draw();
}
ourHandler.on_culling_activate = function(theMenuItem) {
    window.culling = theMenuItem.active;
    ourStatusBar.set("Culling " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}
ourHandler.on_debug_culling_activate = function(theMenuItem) {
    window.debugCulling = theMenuItem.active;
    ourStatusBar.set("Debug culling " + (theMenuItem.active ? "on" : "off"));
    window.queue_draw();
}
ourHandler.on_pause_activate = function(theMenuItem) {
    window.pause = theMenuItem.active;
    ourStatusBar.set("Pause " + (theMenuItem.active ? "on" : "off"));
}
ourHandler.on_statistics_activate = function(theMenuItem) {
    ourStatusBar.set("Statistics " + (theMenuItem.active ? "on" : "off"));
    ourStatusBar.enableStatistics(theMenuItem.active);
    if (theMenuItem.active) {
        window.printStatistics();
    }
    window.queue_draw();
}

//=================================================
// Help Menu Item Handlers
//=================================================

ourHandler.on_about1_activate = function(theMenuItem) {
    var myDialog = ourGlade.get_widget("dlgAbout", ourViewer);
    myDialog.find_child("lblVersion").label =
            "<span size=\"25000\">rendergirl</span>\n" +
            "Revision: " + REVISION_STRING;
    myDialog.show();
}
ourHandler.on_about_closed = function() {
    ourGlade.get_widget("dlgAbout", ourViewer).hide();
}

//=================================================
// Toolbar Handlers
//=================================================

ourHandler.on_picking_trackball_toggled = function(theButton) {
    if (theButton.active) {
        ourGlade.get_widget("picking_trackball").active = true;
    }
}
ourHandler.on_centered_trackball_toggled = function(theButton) {
    if (theButton.active) {
        ourGlade.get_widget("centered_trackball").active = true;
    }
}
ourHandler.on_flymover_toggled = function(theButton) {
    if (theButton.active) {
        ourGlade.get_widget("fly_mover").active = true;
    }
}
ourHandler.on_walkmover_toggled = function(theButton) {
    if (theButton.active) {
        ourGlade.get_widget("walk_mover").active = true;
    }
}

//=================================================
// Main Viewer
//=================================================


if (RenderTest == undefined) var RenderTest = null;

function Viewer(theArguments) {
    this.Constructor(this, theArguments);
}

Viewer.prototype.Constructor = function(self, theArguments) {
    BaseViewer.prototype.Constructor(self, theArguments);
    self.BaseViewer = [];

    self.BaseViewer.setupWindow = self.setupWindow;
    self.setupWindow = function(theRenderWindow, theSetDefaultRenderingCap) {

        self.BaseViewer.setupWindow(theRenderWindow, theSetDefaultRenderingCap);

        var myShaderLibrary = self.getShaderLibrary();
        if (myShaderLibrary) {
            print("Preparing shaderlibrary '" + myShaderLibrary + "'");
            GLResourceManager.prepareShaderLibrary(myShaderLibrary);
        }
        GLResourceManager.prepareShaderLibrary("shaderlibrary.xml");
    }

    self.loadModel = function( theFilename ) {
        print("Loading model '" + theFilename + "'");

        var myScene;
        if (theFilename == null) {
            myScene = new Scene();
            theFilename = "empty scene";
        } else {
            myScene = new Scene(theFilename);
            self.setModelName(theFilename);
            if(fileExists("SCRIPTS/Patch.js")) {
            		print("Using patch 'Patch.js'");
		    				use("Patch.js");
		    				ourPatchObject = new Patch();
		    		}
        }

        myScene.setup();  
        var myCanvas = getDescendantByTagName(myScene.dom, 'canvas', true);
        ourViewer.setScene(myScene, myCanvas);

        if (window.scene.lightsources.childNodesLength() > 2) {
            // disable default lighting
            ourViewer.getLightManager().enableHeadlight(false);
            ourViewer.getLightManager().enableSunlight(false);
            ourStatusBar.set("Opened scene: " + theFilename + ". Default lighting is disabled.");
        } else {
            // enable
            ourViewer.getLightManager().enableHeadlight(true);
            ourViewer.getLightManager().enableSunlight(true);
            ourStatusBar.set("Opened scene: " + theFilename + ". Default lighting is enabled.");
        } 

        ourMaterialTable = ourViewer.applyMaterialTable();
        ourViewer.lastSwitched = {};

        setupGUI();
        
        if(ourPatchObject && typeof(ourPatchObject.onSceneLoaded)=="function") {
    				ourPatchObject.onSceneLoaded();
    		}
        
        window.queue_draw();
    }

    self.onFrame = function(theTime) {

        if (RenderTest) {
            RenderTest.onFrame(theTime);
        }
        
        if(ourPatchObject && typeof(ourPatchObject.onFrame)=="function") {
    				ourPatchObject.onFrame(theTime);
    		}

        ourStatusBar.onFrame();

        var myMover = self.getMover();
        if (myMover) {
            myMover.onFrame(theTime);
        }
        ourAnimationManager.onFrame(theTime);

        window.queue_draw();
    }

    self.onResize = function() {
        // If the statistic is active, the framerate drops dramatically during resize
        ourGlade.get_widget("statistics1").active = false;
    }

    self.onKeyDown = function(theKey) {
        onKey(theKey, true);
    }

    self.onKeyUp = function(theKey) {
        onKey(theKey, false);
    }
    function onKey(theKey, theState) {
        switch (theKey.keyval) {
            case GdkEvent.GDK_F11:
                if (theState) {
                    ourGlade.get_widget("fullscreen_menuitem").active = false;
                }
                break;
            case 50:
                self.getMover().onKey("down", theState);
                break;
            case 52:
                self.getMover().onKey("left", theState);
                break;
            case 54:
                self.getMover().onKey("right", theState);
                break;
            case 56:
                self.getMover().onKey("up", theState);
                break;
        }
    }

		self.onPreRender = function() {        
        if(ourPatchObject && typeof(ourPatchObject.onPreRender)=="function") {
    				ourPatchObject.onPreRender();
    		}
		}

    self.onPostRender = function() {
        if (RenderTest) {
            RenderTest.onPostRender();
        }
        
        if(ourPatchObject && typeof(ourPatchObject.onPostRender)=="function") {
    				ourPatchObject.onPostRender();
    		}
    }

    self.setupSwitchNodeMenu = function() {
        if (_mySwitchNodeMenu) {
            _mySwitchNodeMenu.finalize();
            _mySwitchNodeMenu = null;
            gc();
        }
        _mySwitchNodeMenu = new SwitchNodeMenu();
        _mySwitchNodeMenu.setup( self );
    }

/*
    self.BaseViewer.onMouseButton = self.onMouseButton;
    self.onMouseButton = function(theButton, theState, theX, theY) {
        if (theButton == RIGHT_BUTTON) {
            if (theState) {
                ourCameraPopup.popup(theButton);
            }
        } else {
            self.BaseViewer.onMouseButton(theButton, theState, theX, theY);
        }
    }
*/

    self.finalize = function() {
        _mySwitchNodeMenu.finalize();
    }

    var _mySwitchNodeMenu = null; /*new SwitchNodeMenu( self );*/
}

//===========================================
//  Main Application
//===========================================

var ourCameraComboBox = null;

function setupCameraComboBox() {
    ourCameraComboBox = new ComboBoxText();
    ourGlade.get_widget("camera_box").pack_end(ourCameraComboBox, false, false);
    ourCameraComboBox.show();

    var myCameras = window.scene.cameras;
    if (myCameras.length > 1) {
        for (var i = 0; i < myCameras.length; ++i) {
            ourCameraComboBox.append_text(myCameras[i].name);
        }
        ourCameraComboBox.active_text = window.camera.name;

        ourCameraComboBox.signal_changed.connect(ourCameraComboBox, "on_changed");
        ourCameraComboBox.on_changed = function() {
            for (i = 0; i < window.scene.cameras.length; ++i) {
                if (window.scene.cameras[i].name == this.active_text) {
                    ourViewer.setActiveCamera(window.scene.cameras[i]);
                }
            }
        }
        ourGlade.get_widget("camera_box").show();
    } else {
        ourGlade.get_widget("camera_box").hide();
    }

    ourCameraPopup = ourGlade.get_widget("CameraMenu");
}

function main(argv) {
    try {
    		
    	
    		
    	
        GtkMain.exitCode    = 0;
        ourGlade            = new Glade(GLADE_FILE);
        ourMainWindow       = ourGlade.get_widget("mainWindow");

        ourGlade.autoconnect(ourHandler, ourMainWindow);
        ourHandler.arguments = parseArguments(argv, ourAllowedOptions);
        ourHandler.isLoaded = false;

				ourPreferenceDialog = new PreferenceDialog(ourGlade);
				ourSceneViewerDialog= new SceneViewerDialog(ourGlade, ourHandler);
        ourStatusBar        = new StatusBar(ourGlade.get_widget("statusbar"));
				
        window.renderingCaps = Renderer.MULTITEXTURE_SUPPORT;
        
        ourGlade.get_widget("renderbox").add(window);
        window.show();
        ourMainWindow.show();

        ourViewer = new Viewer(ourHandler.arguments);
        ourMainWindow.signal_key_press_event.connect(ourViewer, "onKeyDown");
        ourMainWindow.signal_key_release_event.connect(ourViewer, "onKeyUp");
        ourMainWindow.signal_delete_event.connect(ourHandler, "on_quit_activate");
        ourViewer.setupWindow(window);
        
        ourViewer.lastSwitched = {};

        /*
        self.setModelName(myFilename);
        var myScene = new Scene(myFilename);
        myScene.setup();        
        //ourViewer.setScene(myScene);
        var myCanvas = getDescendantByTagName(myScene.dom, 'canvas', true);
        //ourViewer.setCanvas(myCanvas);
        self.prepareScene(myScene, myCanvas);
        ourStatusBar.set("Opened scene: " + myFilename);
        setupGUI();
        window.queue_draw();
        */

        ourViewer.registerMover(ClassicTrackballMover);
        ourViewer.registerMover(FlyMover);
        ourViewer.registerMover(WalkMover);
        ourAnimationManager = new GtkAnimationManager(ourViewer);

        ourViewer.loadModel(ourViewer.getModelName());
        //ourViewer.loadModel(null);

        /*
        var myScene = new Scene(ourViewer.getModelName());
        myScene.setup();
        ourViewer.setScene(myScene);
        var myCanvas = getDescendantByTagName(myScene.dom, 'canvas', true);
        ourViewer.setCanvas(myCanvas);
        setupGUI();
        */      

        // enable lighting
        ourViewer.getLightManager().enableHeadlight(true);
        ourViewer.getLightManager().enableSunlight(true);
        
        return GtkMain.run(ourMainWindow);
    } catch (ex) {
        print("### Exception: " + ex);
        return 1;
    }
}


function setupGUI() {
    ourAnimationManager.setup();
    ourViewer.setMover(ClassicTrackballMover, window.canvas.childNode('viewport'));
    ourPreferenceDialog.apply();
    setupCameraComboBox();
    ourViewer.setupSwitchNodeMenu();
}

if (main(arguments) != 0 || GtkMain.exitCode != 0) {
    exit(1);
};
