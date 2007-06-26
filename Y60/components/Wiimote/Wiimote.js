
//=============================================================================
// Copyright (C) 2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================


if (__main__ == undefined) var __main__ = "WiimoteTest";

use("SceneViewer.js");

window = new RenderWindow();

function WiimoteTestApp(theArguments) {
    this.Constructor(this, theArguments);
}



WiimoteTestApp.prototype.Constructor = function(self, theArguments) {

    //////////////////////////////////////////////////////////////////////
    //
    // Constructor
    //
    //////////////////////////////////////////////////////////////////////

    SceneViewer.prototype.Constructor(self, theArguments);
    var Base = [];

    var _myBody = null;
    var _myLowPassedDownVector = new Vector3f( 0, 1, 0 );
    var _myOrientationVector = null;
    
    
    //////////////////////////////////////////////////////////////////////
    //
    // public members
    //
    //////////////////////////////////////////////////////////////////////
   
    // setup
    Base.setup = self.setup;
    self.setup = function(theWidth, theHeight, theTitle) {
        Base.setup(theWidth, theHeight, false, theTitle);
        //window.position = [0, 0];
        window.decorations = false;
        window.resize(theWidth, theHeight);
        myWiimote = plug("Wiimote");


        _myBody = getDescendantByAttribute(window.scene.world, "name", "wii_controller");
        _myBody.scale = new Vector3f(5, 5,5);


        _myOrientationVector = Node.createElement("vector");
        window.scene.world.appendChild( _myOrientationVector );

        _myOrientationVector.color = new Vector4f(1,1,1,1);
        _myOrientationVector.scale = [1,1,1];
        
    }

    Base.onFrame = self.onFrame;
    self.onFrame = function(theTime) {
        Base.onFrame(theTime);

        //_myBody.orientation.assignFromEuler( new Vector3f( 0, 0.1 * theTime, 0));
    }

    Base.onPostRender = self.onPostRender;
    self.onPostRender = function() {
        Base.onPostRender();
    }

    Base.onMouseButton = self.onMouseButton;
    self.onMouseButton = function( theButton, theState, theX, theY ) {
        Base.onMouseButton( theButton, theState, theX, theY);
        print( "mouse pos: " + theX + " " + theY);
    }

    self.onWiiEvent = function( theNode ) {
        //print(theNode);

        if (theNode.type == "button" && theNode.buttonname == "Home" && theNode.pressed == 0) {
            print("Got quit from controller " + theNode.id);
            print("Going home ... good bye!");
            exit( 0 );
        }
        if (theNode.type == "button" && theNode.buttonname == "B") {
            myWiimote.setRumble( theNode.id, theNode.pressed );
        }
        
        if (theNode.type == "motiondata") {
            //print(theNode);
            
            var myDownVector = new Vector3f( theNode.motiondata.x, theNode.motiondata.y,
                                              theNode.motiondata.z );


            myDownVector = normalized( myDownVector );


            _myLowPassedDownVector = normalized( sum( product( _myLowPassedDownVector, 0.9),
                                          product( myDownVector, 0.1) ) );


            _myOrientationVector.value = _myLowPassedDownVector;

            //print("down: " + _myLowPassedDownVector + " magnitude: " + magnitude( _myLowPassedDownVector ));
            _myBody.orientation = new Quaternionf( _myLowPassedDownVector, new Vector3f(0, 1, 0) );
        }

    }
    

    ///////////////////////////////////////////////////////
    // private funtions 
    ///////////////////////////////////////////////////////

    var myWiimote = null;

}


if (__main__ == "WiimoteTest") {
    try {
        var ourWiimoteTestApp = new WiimoteTestApp(arguments);
        //var ourWiimoteTestApp = new WiimoteTestApp(
        //    [expandEnvironment("${PRO}") + "/src/Y60/shader/shaderlibrary_nocg.xml"]);
        //ourWiimoteTestApp.setup(600, 600, "WiimoteTest");
        ourWiimoteTestApp.setup(1400, 1050, "Wiimote");
        ourWiimoteTestApp.go();
    } catch (ex) {
        print("-------------------------------------------------------------------------------");
        print("### Error: " + ex);
        print("-------------------------------------------------------------------------------");
        exit(1);
    }
}



