//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: DemoRendererExtension.js,v $
//   $Author: christian $
//   $Revision: 1.6 $
//   $Date: 2004/10/31 15:58:17 $
//
//
//=============================================================================

use("SceneViewer.js");
//plug("EdgeBlender");


function EdgeBlendViewer(theArguments) {
    this.Constructor(this, theArguments);
}

EdgeBlendViewer.prototype.Constructor = function(self, theArguments) {

    //////////////////////////////////////////////////////////////////////
    //
    // Constructor
    //
    //////////////////////////////////////////////////////////////////////

    SceneViewer.prototype.Constructor(self, theArguments);
    var Base = [];
    var _myEdgeBlender = null; //new EdgeBlender();

    //////////////////////////////////////////////////////////////////////
    //
    // public members
    //
    //////////////////////////////////////////////////////////////////////

    // setup
    /*
    Base.setup = self.setup;
    self.setup = function(theWidth, theHeight, theTitle) {
        Base.setup(theWidth, theHeight, false, theTitle);
        window.addExtension(_myEdgeBlender);
        self.registerSettingsListener(_myEdgeBlender, "EdgeBlender");
    }*/


    Base.setup = self.setup;
    self.setup = function(theWidth, theHeight, theFullscreen, theTitle) {
        window = new RenderWindow();
        window.decorations = false;
        window.position = new Vector2i(0,0);

        var myCamera = getDescendantByTagName(self.getWorld(), "camera", true);
        //self.setActiveCamera(myCamera);

        self.setSplashScreen(false);
        Base.setup(theWidth, theHeight, theFullscreen, theTitle);

        //self.setMover(null);

        //window.addExtension(_myEdgeBlender);
        //self.registerSettingsListener(_myEdgeBlender, "EdgeBlender");
        
        // clone viewport
        _myViewport1 = getDescendantByTagName(window.canvas, "viewport");
        _myViewport1.position = new Vector2f(-0.1,0);
        
        if (_myViewport1) {
            print("Cloning viewport '" + _myViewport1.name + "'");
            _myViewport2 = _myViewport1.cloneNode(true);
            _myViewport2.position = new Vector2f(0.1,0);
            adjustNodeId(_myViewport2, true);
            _myViewport2.name = "Left";
            window.canvas.appendChild(_myViewport2);
        }
        window.scene.save("buf.x60");
        
    }

    Base.onKey = self.onKey;
    self.onKey = function(theKey, theState, theX, theY, theShiftFlag, theCtrlFlag, theAltFlag) {
        if (theCtrlFlag) {
            Base.onKey(theKey, theState, theX, theY, theShiftFlag, theCtrlFlag, theAltFlag);
            return;
        }
        if (theState) {
            if (theShiftFlag) {
                theKey = theKey.toUpper();
            }
            switch (theKey) {
                case 'a':
                    _myEdgeBlender.blendwidth += 0.01;
                    break;
                case 'd':
                    _myEdgeBlender.blendwidth -= 0.01;
                    break;
                case 'w':
                    _myEdgeBlender.blacklevel += 0.01;
                    break;
                case 's':
                    _myEdgeBlender.blacklevel -= 0.01;
                    break;
                case 'q':
                    _myEdgeBlender.mode = (_myEdgeBlender.mode + 1) % 3;
                    break;
                case 'y':
                    _myEdgeBlender.subdivisions += 1;
                    break;
                case 'c':
                    _myEdgeBlender.subdivisions -= 1;
                    break;
                case 'h':
                    print("EdgeBlend Help:");
                    print("a/d Increase/decrease blend width");
                    print("w/s Increase/decrease black level");
                    print("q   Cycle left/right/both blend out mode");
                    break;
                default:
            }
        }
    }

    Base.onFrame = self.onFrame;
    self.onFrame = function(theTime) {
        Base.onFrame(theTime);

    }

    var _myViewport1, _myViewport2;

}

//
// main
//

try {
    var ourApp = new EdgeBlendViewer(arguments);

    ourApp.setup(2048, 800, false, "EdgeBlendViewer");
    ourApp.go();
} catch (ex) {
    print("-------------------------------------------------------------------------------");
    print("### Error: " + ex);
    print("-------------------------------------------------------------------------------");
    exit(1);
}


