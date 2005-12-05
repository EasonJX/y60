//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
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

// TODO:
// - Test parent handling

use("UnitTest.js");
use("OverlayManager.js");
use("Overlay.js");
use("Y60JSSL.js");

var window = null;

function OverlayUnitTest() {
    this.Constructor(this, "OverlayUnitTest");
}

OverlayUnitTest.prototype.Constructor = function(obj, theName) {

    UnitTest.prototype.Constructor(obj, theName);

    obj.myDummyOverlay = null;
    obj.myScene     = null;

    obj.getAttribute = function(theNode, theAttributeName) {
        var myNode = new Node(theNode).firstChild;
        return myNode[theAttributeName];
        /*
        var myRegExp = new RegExp(theAttributeName + '=\\"(.\\w)\\"');
        myRegExp.exec(String(theNode));
        return RegExp.$1;*/
    }

    obj.getLastOverlay = function() {
        return getDescendantByTagName(obj.myScene, "overlays", true).lastChild;
    }

    function testCommonProperties(thePosition, theSize, theColor) {
        // Test toString operator
        obj.myOverlayString = String(obj.myOverlay);
        ENSURE('obj.myOverlayString.substr(0,9) == "<overlay "');
        ENSURE('obj.myOverlay.color == ' + theColor);

        // Test material convenience property
        obj.myMaterialNode = obj.myScene.getElementById(obj.getAttribute(obj.myOverlay, "material"));
        ENSURE('obj.myOverlay.material.id == "' + obj.myMaterialNode.id + '"');
        obj.myOverlay.material = obj.myDummyOverlay.material;

        ENSURE('obj.myOverlay.material.id == obj.myDummyOverlay.material.id');
        ENSURE('obj.getAttribute(obj.myOverlay, "material") == obj.myDummyOverlay.material.id');
        ENSURE('obj.myOverlay.color == 1,1,1,1');
        obj.myOverlay.material = obj.myMaterialNode;

        // Test color convenience property
        obj.myOverlay.color = new Vector4f(4,3,2,1);
        obj.myColor = getDescendantByName(obj.myOverlay.material, "surfacecolor", true).firstChild.nodeValue;
        ENSURE('almostEqual(obj.myOverlay.color, obj.myColor)');

        // Test move to top
        ENSURE('String(obj.myOverlay) == String(obj.getLastOverlay())');
        obj.myDummyOverlay.moveToTop();
        ENSURE('String(obj.myOverlay) != String(obj.getLastOverlay())');
        ENSURE('String(obj.myDummyOverlay) == String(obj.getLastOverlay())');
        obj.myOverlay.moveToTop();

        // Paranoid touches test
        ENSURE(obj.myOverlay.touches(thePosition[0], thePosition[1]));
        ENSURE(!obj.myOverlay.touches(thePosition[0] - 1, thePosition[1]));
        ENSURE(!obj.myOverlay.touches(thePosition[0], thePosition[1] - 1));
        ENSURE(obj.myOverlay.touches(thePosition[0] + theSize[0] - 1, thePosition[1] + theSize[1] - 1));
        ENSURE(!obj.myOverlay.touches(thePosition[0] + theSize[0] - 1, thePosition[1] + theSize[1]));
        ENSURE(!obj.myOverlay.touches(thePosition[0] + theSize[0], thePosition[1] + theSize[1] - 1));
        ENSURE(obj.myOverlay.touches(thePosition[0], thePosition[1], 0));
        ENSURE(!obj.myOverlay.touches(thePosition[0] - 1, thePosition[1], 0));
        ENSURE(!obj.myOverlay.touches(thePosition[0], thePosition[1] - 1, 0));
        ENSURE(obj.myOverlay.touches(thePosition[0] + theSize[0] - 1, thePosition[1] + theSize[1] - 1, 0));
        ENSURE(!obj.myOverlay.touches(thePosition[0] + theSize[0] - 1, thePosition[1] + theSize[1], 0));
        ENSURE(!obj.myOverlay.touches(thePosition[0] + theSize[0], thePosition[1] + theSize[1] - 1, 0));
        ENSURE(obj.myOverlay.touches(thePosition[0] - 1, thePosition[1] - 1, 1));
        ENSURE(!obj.myOverlay.touches(thePosition[0] - 2, thePosition[1] - 1, 1));
        ENSURE(!obj.myOverlay.touches(thePosition[0] - 1, thePosition[1] - 2, 1));
        ENSURE(obj.myOverlay.touches(thePosition[0] + theSize[0], thePosition[1] + theSize[1], 1));
        ENSURE(!obj.myOverlay.touches(thePosition[0] + theSize[0] + 1, thePosition[1] + theSize[1], 1));
        ENSURE(!obj.myOverlay.touches(thePosition[0] + theSize[0], thePosition[1] + theSize[1] + 1, 1));

        ///////////////////////////////////////////////////////////////////////////////////////
        //
        // Test attribute access
        //
        ///////////////////////////////////////////////////////////////////////////////////////

        obj.myOverlay.name = "nina";
        ENSURE('obj.myOverlay.name == "nina"');

        ENSURE('obj.myOverlay.visible');
        obj.myOverlay.visible = false;
        ENSURE('!obj.myOverlay.visible');

        ENSURE('almostEqual(obj.myOverlay.position, ' + arrayToString(thePosition) + ')');
        obj.myOverlay.position = new Vector2f(2,1);
        ENSURE('almostEqual(obj.myOverlay.position, [2,1])');

        ENSURE('almostEqual(obj.myOverlay.srcorigin, [0,0])');
        obj.myOverlay.srcorigin = new Vector2f(1,2);
        ENSURE('almostEqual(obj.myOverlay.srcorigin, [1,2])');

        ENSURE('almostEqual(obj.myOverlay.srcsize, [1,1])');
        obj.myOverlay.srcsize = new Vector2f(3,4);
        ENSURE('almostEqual(obj.myOverlay.srcsize, [3,4])');

        ENSURE('obj.myOverlay.alpha == 1');
        obj.myOverlay.alpha = 0.5;
        ENSURE('obj.myOverlay.alpha == 0.5');

        ENSURE('obj.myOverlay.width == ' + theSize[0]);
        obj.myOverlay.width = 30;
        ENSURE('obj.myOverlay.width == 30');

        ENSURE('obj.myOverlay.height == ' + theSize[1]);
        obj.myOverlay.height = 10;
        ENSURE('obj.myOverlay.height == 10');

        ENSURE('almostEqual(obj.myOverlay.bordercolor, [0,0,0,1])');
        obj.myOverlay.bordercolor = new Vector4f(1,2,3,4);
        ENSURE('almostEqual(obj.myOverlay.bordercolor, [1,2,3,4])');

        ENSURE('obj.myOverlay.borderwidth == 1');
        obj.myOverlay.borderwidth = 3;
        ENSURE('obj.myOverlay.borderwidth == 3');

        ENSURE('!obj.myOverlay.topborder');
        obj.myOverlay.topborder = true;
        ENSURE('obj.myOverlay.topborder');

        ENSURE('!obj.myOverlay.bottomborder');
        obj.myOverlay.bottomborder = true;
        ENSURE('obj.myOverlay.bottomborder');

        ENSURE('!obj.myOverlay.leftborder');
        obj.myOverlay.leftborder = true;
        ENSURE('obj.myOverlay.leftborder');

        ENSURE('!obj.myOverlay.rightborder');
        obj.myOverlay.rightborder = true;
        ENSURE('obj.myOverlay.rightborder');
    }

    function testRemoveFromScene() {
        ENSURE('obj.myScene.getElementById(obj.myOverlay.material.id) != undefined');
        ENSURE('obj.myScene.getElementById(obj.myOverlay.image.id) != undefined');
        ENSURE('String(obj.myOverlay) == String(obj.getLastOverlay())');
        obj.myOverlay.removeFromScene();
        ENSURE('obj.myScene.getElementById(obj.myOverlay.material.id) == undefined');
        ENSURE('obj.myScene.getElementById(obj.myOverlay.image.id) == undefined');
        ENSURE('String(obj.myOverlay) != String(obj.getLastOverlay())');
    }

    obj.run = function() {
        GLResourceManager.loadShaderLibrary("../../../../shader/shaderlibrary.xml");
        // Create empty scene
        window = new RenderWindow();

        obj.myScene = window.scene.dom;
        var myViewport = getDescendantByTagName(obj.myScene, "viewport", true);
        var myOverlayManager = new OverlayManager(window.scene, myViewport);
        obj.myDummyOverlay = new ImageOverlay(myOverlayManager, "../../testfiles/black.rgb");
        obj.myDummyImageId = getDescendantByTagName(obj.myScene, "images").lastChild.id;
        obj.myOverlay = new Overlay(myOverlayManager, new Vector4f(1,2,3,1), [10, 20], [100,200]);
        //testCommonProperties([10,20], [100,200], [1,2,3,1]);

        obj.myOverlay = new ImageOverlay(myOverlayManager, "../../testfiles/DiffuseRamp.png", [30, 40]);
        obj.myImageId = getDescendantByTagName(obj.myScene, "images").lastChild.id;
        //testCommonProperties([30,40], [32,1], [1,1,1,1]);

        ///////////////////////////////////////////////////////////////////////////////////////
        //
        // Test multiple image support
        //
        ///////////////////////////////////////////////////////////////////////////////////////


        var mySources = ["../../testfiles/black.rgb", "../../testfiles/DiffuseRamp.png"];
        obj.myMultiOverlay = new ImageOverlay(myOverlayManager, mySources);
        ENSURE('obj.myMultiOverlay.images.length == 2');

        obj.myMultiOverlay.position = new Vector2f(300,300);

        var myImages = obj.myMultiOverlay.images;
        myImages.push( obj.myMultiOverlay.images[0].cloneNode(true) );
        // the following wont work, we modified images in place
        // ENSURE('obj.myMultiOverlay.images.length == 2');
        obj.myMultiOverlay.images = myImages;
        ENSURE('obj.myMultiOverlay.images.length == 3');
        myImages = [];
        myImages.push(obj.myMultiOverlay.images[0]);
        obj.myMultiOverlay.images = myImages;
        ENSURE('obj.myMultiOverlay.images.length == 1');




        ///////////////////////////////////////////////////////////////////////////////////////
        //
        // Test texture and image convenience property
        //
        ///////////////////////////////////////////////////////////////////////////////////////

        obj.myMaterialNode = obj.myScene.getElementById(obj.getAttribute(obj.myOverlay, "material"));

        ENSURE('obj.myOverlay.texture.image == obj.myOverlay.image.id');
        obj.myTextureNode = getDescendantByTagName(obj.myMaterialNode, "texture", true);
        ENSURE('obj.myTextureNode.image == obj.myOverlay.texture.image');
        ENSURE('obj.myTextureNode.image == obj.myOverlay.image.id');

        ENSURE('String(obj.myMaterialNode) == String(obj.myOverlay.material)');
        ENSURE('String(obj.myTextureNode) == String(obj.myOverlay.texture)');
        obj.myOverlay.texture = obj.myDummyOverlay.texture;
        ENSURE('String(obj.myMaterialNode) == String(obj.myOverlay.material)');
        obj.myTextureNode = getDescendantByTagName(obj.myMaterialNode, "texture", true);
        ENSURE('String(obj.myTextureNode) == String(obj.myOverlay.texture)');

        ENSURE('obj.myOverlay.texture.image == obj.myDummyImageId');
        ENSURE('obj.myOverlay.image.id == obj.myDummyImageId');
        ENSURE('obj.myTextureNode.image == obj.myDummyImageId');
        obj.myOverlay.image = obj.myScene.getElementById(obj.myImageId);
        ENSURE('obj.myOverlay.texture.image == obj.myImageId');
        ENSURE('obj.myOverlay.image.id == obj.myImageId');
        ENSURE('obj.myTextureNode.image == obj.myImageId');

        obj.myOverlay.image = obj.myDummyOverlay.image;
        ENSURE('obj.myOverlay.texture.image == obj.myDummyImageId');
        ENSURE('obj.myOverlay.image.id == obj.myDummyImageId');
        ENSURE('obj.myTextureNode.image == obj.myDummyImageId');

        obj.myOverlay = new MovieOverlay(myOverlayManager, "../../testfiles/testmovie.m60", [40, 50], null, false);
        Logger.info(obj.myOverlay.movie);
        ENSURE('obj.myOverlay.movie.audio == false');
        ENSURE('obj.myOverlay.movie.framecount == 30');
        testCommonProperties([40,50], [480,16], [1,1,1,1]);
        testRemoveFromScene();
    }
}

function main() {
    var myTestName = "testOverlay.tst.js";
    var mySuite = new UnitTestSuite(myTestName);

    mySuite.addTest(new OverlayUnitTest());
    mySuite.run();

    print(">> Finished test suite '"+myTestName+"', return status = " + mySuite.returnStatus() + "");
    return mySuite.returnStatus();
}

if (main() != 0) {
    exit(1);
}

