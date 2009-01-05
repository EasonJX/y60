/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2008, ART+COM AG Berlin, Germany <www.artcom.de>
//
// These coded instructions, statements, and computer programs contain
// proprietary information of ART+COM AG Berlin, and are copy protected
// by law. They may be used, modified and redistributed under the terms
// of GNU General Public License referenced below. 
//    
// Alternative licensing without the obligations of the GPL is
// available upon request.
//
// GPL v3 Licensing:
//
// This file is part of the ART+COM Y60 Platform.
//
// ART+COM Y60 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ART+COM Y60 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ART+COM Y60.  If not, see <http://www.gnu.org/licenses/>.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Description: TODO  
//
// Last Review: NEVER, NOONE
//
//  review status report: (perfect, ok, fair, poor, disaster, notapplicable, unknown)
//    usefullness            : unknown
//    formatting             : unknown
//    documentation          : unknown
//    test coverage          : unknown
//    names                  : unknown
//    style guide conformance: unknown
//    technical soundness    : unknown
//    dead code              : unknown
//    readability            : unknown
//    understandabilty       : unknown
//    interfaces             : unknown
//    confidence             : unknown
//    integration            : unknown
//    dependencies           : unknown
//    cheesyness             : unknown
//
//    overall review status  : unknown
//
//    recommendations: 
//       - unknown
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __

*/

use("UnitTest.js");

function JSAppUnitTest() {
    this.Constructor(this, "JSAppUnitTest");
};

JSAppUnitTest.prototype.Constructor = function(obj, theName) {

    UnitTest.prototype.Constructor(obj, theName);

    obj.run = function() {
        ENSURE('"pickytests" == "pickytests"');
        /* [DS] disabled because of issues with the new continous build:
         *      - continous build now runs under cygwin
         *      - the test seems not to find the 'exec.bat' script
         *      - I guess this is because of working directory confusion
         */
        /*
        obj.myReturnCode = exec("../../testfiles/exec.sh", "2");
        ENSURE('obj.myReturnCode == 2');
        obj.myReturnCode = exec("../../testfiles/exec.sh 3");
        ENSURE('obj.myReturnCode == 3');
        obj.myReturnCode = exec("../../testfiles/test.butt");
        ENSURE('obj.myReturnCode == -1');
        obj.myReturnCode = exec("../../testfiles/exec.sh", "2", false);
        ENSURE('obj.myReturnCode == 0');
        if (operatingSystem() == "WIN32") {
            obj.myReturnCode = exec("..\\..\\testfiles\\exec.bat", "2");
    		ENSURE('obj.myReturnCode == 2');
            obj.myReturnCode = exec("..\\..\\testfiles\\exec.bat 3");
    		ENSURE('obj.myReturnCode == 3');
            obj.myReturnCode = exec("..\\..\\testfiles\\test.butt");
    		ENSURE('obj.myReturnCode == -1');
            obj.myReturnCode = exec("..\\..\\testfiles\\exec.bat", "2", false);
    		ENSURE('obj.myReturnCode == 0');
        } else if (operatingSystem() == "LINUX") {
            ENSURE('"pickytests" == "pickytests"');
        } else if (operatingSystem() == "OSX") {
            ENSURE('"pickytests" == "pickytests"');
        }
        */
        
    }
};

try {
    var myTestName = "testJSapp.tst.js";
    var mySuite = new UnitTestSuite(myTestName);

    mySuite.addTest(new JSAppUnitTest());

	mySuite.run();

    print(">> Finished test suite '"+myTestName+"', return status = " + mySuite.returnStatus() + "");
    exit(mySuite.returnStatus());
} catch (ex) {
    print("-----------------------------------------------------------------------------------------");
    print("### Error: " + ex);
    print("-----------------------------------------------------------------------------------------");
    exit(1);
}
