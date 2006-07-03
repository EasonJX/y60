//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

use("BaseViewer.js");
use("PreferenceDialog.js");
use("DebugVisual.js");
use("ClassicTrackballMover.js");
use("FlyMover.js");
use("WalkMover.js");
use("GUIUtils.js");
use("SwitchNodeMenu.js");

var ourHandler           = {};

use("StatusBar.js");
use("GtkAnimationManager.js");

const REVISION_STRING    = revision();

var window               = new RenderArea();

var ourGlade             = null;
var ourViewer            = null;
var ourMainWindow        = null
var ourPreferenceDialog  = null;
var ourCameraPopup       = null;
var ourAnimationManager  = null;

var ourCoordinateSystem  = null;
var ourStatusBar         = null;

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

//=================================================
// File Menu Item Handlers
//=================================================

ourHandler.on_new_activate = function() {
    ourViewer.setModelName("");
    var myScene = new Scene(null);
    myScene.setup();
    ourViewer.setScene(myScene);
    var myCanvas = getDescendantByTagName(myScene.dom, 'canvas', true);
    ourViewer.setCanvas(myCanvas);
    ourStatusBar.set("New scene");
    setupGUI();
    window.queue_draw();
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
        ourViewer.setModelName(myFilename);
        var myScene = new Scene(myFilename);
        myScene.setup();        
        //ourViewer.setScene(myScene);
        var myCanvas = getDescendantByTagName(myScene.dom, 'canvas', true);
        //ourViewer.setCanvas(myCanvas);
        ourViewer.prepareScene(myScene, myCanvas);
        ourStatusBar.set("Opened scene: " + myFilename);
        setupGUI();
        window.queue_draw();
    }
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

var ourWindowState = {width: 800, height: 600, position: [0, 0]};
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
    var myHfov = window.camera.hfov;
    if (myHfov > 5) {
        myHfov -= 5;
    } else if (myHfov > 0.1) {
        myHfov -= 0.1;
    }
    window.camera.hfov = myHfov;
    ourStatusBar.set("Zoom to " + getFocalLength(myHfov).toFixed(1) + "mm (HFOV: " + myHfov.toFixed(1) + ")");
    window.queue_draw();
}

ourHandler.on_zoom_out_activate = function() {
    var myHfov = window.camera.hfov;
    if (myHfov < 5) {
        myHfov += 0.1;
    } else if (myHfov < 175) {
        myHfov += 5;
    }
    window.camera.hfov = myHfov;
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
    Logger.warning("Quit");
    ourViewer.finalize();
    GtkMain.quit();
}

//=================================================
// Tools Menu Item Handlers
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

    self.onFrame = function(theTime) {
        if (RenderTest) {
            RenderTest.onFrame(theTime);
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


    self.onPostRender = function() {
        if (RenderTest) {
            RenderTest.onPostRender();
        }
    }

    self.setupSwitchNodeMenu = function() {
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

    var _mySwitchNodeMenu = new SwitchNodeMenu( self );
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

        ourPreferenceDialog = new PreferenceDialog(ourGlade);
        ourStatusBar        = new StatusBar(ourGlade.get_widget("statusbar"));

        ourGlade.autoconnect(ourHandler, ourMainWindow);
        ourHandler.arguments = parseArguments(argv, ourAllowedOptions);
        ourHandler.isLoaded = false;

        window.renderingCaps = Renderer.MULTITEXTURE_SUPPORT;
        ourGlade.get_widget("renderbox").add(window);
        window.show();
        ourMainWindow.show();
        ourViewer = new Viewer(ourHandler.arguments);
        ourMainWindow.signal_key_press_event.connect(ourViewer, "onKeyDown");
        ourMainWindow.signal_key_release_event.connect(ourViewer, "onKeyUp");
        ourMainWindow.signal_delete_event.connect(ourHandler, "on_quit_activate");
        ourViewer.setupWindow(window);

        var myScene = new Scene(ourViewer.getModelName());
        myScene.setup();
        ourViewer.setScene(myScene);
        var myCanvas = getDescendantByTagName(myScene.dom, 'canvas', true);
        ourViewer.setCanvas(myCanvas);
        ourViewer.registerMover(ClassicTrackballMover);
        ourViewer.registerMover(FlyMover);
        ourViewer.registerMover(WalkMover);
        ourAnimationManager = new GtkAnimationManager(ourViewer);
        setupGUI();

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


