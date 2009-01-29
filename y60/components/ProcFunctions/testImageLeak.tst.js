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

use("Overlay.js");
use("Exception.js");
use("UnitTest.js");

plug("y60ProcFunctions");

const ALLOWED_MEMORY_USAGE = 3*1024*1024;

const START_FRAMES = 10;
const END_FRAMES   = 10;
const IMAGE_COUNT  = 1000;

const IMAGE_1 = expandEnvironment("${PRO}/src/y60/shader/shadertex/ac_logo.png");
const IMAGE_2 = expandEnvironment("${PRO}/src/y60/shader/shadertex/hatch.jpg");
var myShaderLibrary = "${PRO}/src/y60/shader/shaderlibrary.xml";
GLResourceManager.prepareShaderLibrary(myShaderLibrary);

var window     = new RenderWindow();
window.fixedFrameTime = 0.04;

function ImageLeakUnitTest() {
    this.Constructor(this, "ImageLeakUnitTest");
};

ImageLeakUnitTest.prototype.Constructor = function(obj, theName) {

    var _myToggle = true;
    var _myOverlay = null;
    var _myFrameCount = 0;
    var _myStartMemory = 0;
    UnitTest.prototype.Constructor(obj, theName);

    obj.run = function() {
        window.onFrame = function(theTime) {
            if (_myFrameCount == START_FRAMES) {
                _myStartMemory = getProcessMemoryUsage();
            } else if (_myFrameCount > START_FRAMES && _myFrameCount < START_FRAMES + IMAGE_COUNT) {
                toggleImage();
            } else if (_myFrameCount == START_FRAMES + IMAGE_COUNT) {
                remove();
            } else if (_myFrameCount >= START_FRAMES + IMAGE_COUNT + END_FRAMES) {
                gc();
                var myUsedMemory = getProcessMemoryUsage();

                // must be attached to obj for ENSURE to work
                obj.myMemoryDifff =  myUsedMemory - _myStartMemory;
                obj.myAllowedMemoryUsage = ALLOWED_MEMORY_USAGE;

                print("Number of textures                      : " + window.scene.textures.childNodesLength());
                print("Number of images                        : " + window.scene.images.childNodesLength());
                print("Memory at first image construction time : " + _myStartMemory);
                print("Memory at app end                       : " + myUsedMemory);
                print("Difference                              : " + obj.myMemoryDifff);
                print("allowed difference                      : " + obj.myAllowedMemoryUsage + " ,(due to some basic memory allocation, i.e. SomImageFactory)");
                ENSURE("obj.myMemoryDifff < obj.myAllowedMemoryUsage");
                ENSURE("window.scene.textures.childNodesLength() == 0");
                ENSURE("window.scene.images.childNodesLength() == 0");
                window.stop();
            }
            _myFrameCount++;
        }
        window.go();
    }
    function remove() {
        if (_myOverlay != null) {
            _myOverlay.removeFromScene();
            _myOverlay = null;
            gc();
        }
    }
    function toggleImage() {
        remove();
        if (_myToggle) {
            _myOverlay = new ImageOverlay(window.scene, IMAGE_1);
        } else {
            _myOverlay = new ImageOverlay(window.scene, IMAGE_2);
        }
        _myToggle = !_myToggle;
    }
}

var myTestName = "ImageLeakUnitTest.tst.js";
var mySuite = new UnitTestSuite(myTestName);

mySuite.addTest(new ImageLeakUnitTest());
mySuite.run();

print(">> Finished test suite '"+myTestName+"', return status = " + mySuite.returnStatus() + "");
exit(mySuite.returnStatus());

