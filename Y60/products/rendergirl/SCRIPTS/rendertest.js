//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

var RenderTest = {};
var ourFrameCounter = 0;
var ourSavedFrameCounter = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// All our tests
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

var ourTests = [];
ourTests.push({title: "Initial Image", screenshot: true});

// File Menu Item Handlers
ourTests.push({title: "Save", handler: "on_save_activate"});
ourTests.push({title: "Save as", handler: "on_save_as_activate", arguments: "TESTMODELS/test_save_as.x60"});
ourTests.push({title: "New File", handler: "on_new_activate", screenshot: true});
ourTests.push({title: "Open", handler: "on_open_activate", arguments: "TESTMODELS/test_save_as.x60", screenshot: true});
ourTests.push({title: "Saved screenshot"}); // Just to change the title
ourTests.push({title: "Save screenshot", handler: "on_save_screenshot_activate", arguments: "TEST_IMAGES/saved_image.png"});

// Edit Menu Item Handlers
ourTests.push({title: "Open Preferences", handler: "on_preferences_activate"});
ourTests.push({title: "Close Preferences", callback: "closePreferences()"});

// View Menu Item Handlers
ourTests.push({title: "Wireframe", handler: "on_wireframe_activate", active: true, screenshot: true});
ourTests.push({title: "Texturing off", handler: "on_texturing_activate", active: false, screenshot: true});
ourTests.push({title: "Flatshading on", handler: "on_flatshading_activate", active: true, screenshot: true});
ourTests.push({title: "Backface culling on", handler: "on_backface_culling_activate", active: true, callback: "moveCameraToCenter()", screenshot: true});
ourTests.push({title: "Backface culling off", handler: "on_backface_culling_activate", active: false,  screenshot: true});
ourTests.push({title: "Reset camera", handler: "on_reset_camera_activate", screenshot: true});
//ourTests.push({title: "Coordinate system on", handler: "on_coordinate_system_activate", active: true, screenshot: true});
// on_fullscreen_activate does not test easily

// Camera Menu Item Handlers
ourTests.push({title: "Previous camera", handler: "on_previous_camera_activate", screenshot: true});
ourTests.push({title: "Next camera", handler: "on_next_camera_activate", screenshot: true});
ourTests.push({title: "Picking trackball mover", handler: "on_picking_trackball_activate", active: true});
ourTests.push({title: "Centered trackball mover", handler: "on_centered_trackball_activate", active: true});
ourTests.push({title: "Fly mover", handler: "on_fly_mover_activate", active: true});
ourTests.push({title: "Walk mover", handler: "on_walk_mover_activate", active: true});
ourTests.push({title: "Zoom in", handler: "on_zoom_in_activate", screenshot: true});
ourTests.push({title: "Zoom out", handler: "on_zoom_out_activate", screenshot: true});
ourTests.push({title: "Reset camera", handler: "on_reset_camera_activate"});

// Light Menu Item Handlers
ourTests.push({title: "Lighting off", handler: "on_lighting_activate", active: false, screenshot: true});
ourTests.push({title: "Headlight off", handler: "on_headlight_activate", active: false, screenshot: true});
ourTests.push({title: "Sunlight off", handler: "on_sunlight_activate", active: false, screenshot: true});

// Tools Menu Item Handlers
ourTests.push({title: "Body bounding boxes", handler: "on_bb_body_activate", active: false, screenshot: true});
ourTests.push({title: "Shape bounding boxes", handler: "on_bb_shape_activate", active: false, screenshot: true});
ourTests.push({title: "Hierarchy bounding boxes", handler: "on_hierarchy_activate", active: false, screenshot: true});
ourTests.push({title: "Bounding boxes off", handler: "on_bb_off_activate", active: false, screenshot: true});
ourTests.push({title: "Culling on", handler: "on_culling_activate", active: true});
ourTests.push({title: "Debug culling on", handler: "on_debug_culling_activate", active: true});
ourTests.push({title: "Pause on", handler: "on_pause_activate", active: true, callback: "ourHandler.on_pause_activate({active:false})"});
ourTests.push({title: "Statistics on", handler: "on_statistics_activate", active: true});

// Help Menu Item Handlers
ourTests.push({title: "About box", handler: "on_about1_activate", active: true});
ourTests.push({title: "Close About box", handler: "on_about_closed", active: true});

// Animation Menu Item Handlers
ourTests.push({title: "Run animations", handler: "on_run_animations_activate", active: true, screenshot: true});
ourTests.push({title: "Stop animations", handler: "on_run_animations_activate", active: false, screenshot: true});
ourTests.push({title: "Animations step forward", handler: "on_animation_step_forward_activate", screenshot: true});
ourTests.push({title: "Animations step backard", handler: "on_step_backward_activate", screenshot: true});
ourTests.push({title: "Reset animations", handler: "on_reset_animations_activate", screenshot: true});

// And finally the quit handler :-)
ourTests.push({title: "Quit", handler: "on_quit_activate"});

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Some helper callbacks
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

function closePreferences() {
    ourPreferenceDialog.onResponse(Dialog.RESPONSE_OK);
}

function moveCameraToCenter() {
    window.camera.position = new Vector3f(0, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// The test framework
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

RenderTest.onFrame = function(theTime) {
    try {
        window.fixedFrameTime = 0.1;
        var myTest = ourTests[ourFrameCounter];
        var myLastTest = ourFrameCounter ? ourTests[ourFrameCounter - 1] : null;
        print(ourFrameCounter + ". " + myTest.title);

        // Deactivate last test
        if (myLastTest && "handler" in myLastTest && "active" in myLastTest) {
            myLastTest.active = !myLastTest.active;
            ourHandler[myLastTest.handler](myLastTest);
        }

        // Activate current test
        if ("handler" in myTest) {
            var myArguments = myTest;
            if ("arguments" in myTest) {
                myArguments = myTest.arguments;
            }
            ourHandler[myTest.handler](myArguments);
        }

        if ("callback" in myTest) {
            eval(myTest.callback);
        }

        window.renderText([30, 500], ourFrameCounter + ". " + myTest.title);
    } catch (ex) {
        reportException(ex);
        GtkMain.exitCode = 1;
        GtkMain.quit();
    }
}

RenderTest.onPostRender = function(theTime) {
    var myTest = ourTests[ourFrameCounter];
    if ("screenshot" in myTest && myTest.screenshot) {
        //print(">>> Save: '" + myTest.title + "' as 'cube_" + ourFrameCounter + ".png'");
        var myNumber = ourFrameCounter < 10 ? "0" + ourFrameCounter : ourFrameCounter;
        window.saveBuffer("TEST_IMAGES/cube_" + myNumber + ".png");
    }
    ourFrameCounter++;

    if (ourFrameCounter >= ourTests.length) {
        GtkMain.quit();
    }
}

// Start the rendergirl application
use("rendergirl.js");

