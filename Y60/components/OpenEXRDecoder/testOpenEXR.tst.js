/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2005, ART+COM Berlin GmbH
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
//    $RCSfile: testOpenEXR.tst.js,v $
//
//   $Revision: 1.5 $
//
// Description: Utility Classes and Macros for easy unit testing
//
//
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/

use("UnitTest.js");

function OpenEXRUnitTest() {
    this.Constructor(this, "OpenEXRUnitTest");
};


OpenEXRUnitTest.prototype.Constructor = function(obj, theName) {
    UnitTest.prototype.Constructor(obj, theName);

    obj.run = function() {
        plug("y60OpenEXR");

        var window = new RenderWindow();

        var myImage = window.scene.images.appendChild(new Node("<image/>").firstChild);
        //myImage.src = expandEnvironment("${PRO}/testmodels/tex/testbild01.rgb");
        myImage.src = "../../testimages/Desk.exr";
        print(myImage);
        window.scene.update(Scene.IMAGES);

        obj.myVar = 1;
        ENSURE('obj.myVar == 1');
        ENSURE('1 + 1 == 2');
    }
};

var myTestName = "testOpenEXR.tst.js";
var mySuite = new UnitTestSuite(myTestName);

mySuite.addTest(new OpenEXRUnitTest());
mySuite.run();

print(">> Finished test suite '"+myTestName+"', return status = " + mySuite.returnStatus() + "");
exit(mySuite.returnStatus());
