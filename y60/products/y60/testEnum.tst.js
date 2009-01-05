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

function TestEnum() {
    this.Constructor(this, "TestEnum");
};

TestEnum.prototype.Constructor = function(obj, theName) {

    UnitTest.prototype.Constructor(obj, theName);

    obj.run = function() {
        obj.myEnum = BlendEquation.max;
        ENSURE('obj.myEnum == BlendEquation.max');

        switch (obj.myEnum) {
            case BlendEquation.max:
                SUCCESS("switch");
                break;
            default:
                FAILURE("switch");
        }

        ENSURE('obj.myEnum == "max"');
        obj.myScene = new Scene();
        obj.myScene.setup();
        obj.myMaterial = Modelling.createUnlitTexturedMaterial(obj.myScene, "", "Foo", false);
        ENSURE("obj.myMaterial.properties.blendequation == BlendEquation.add");
        obj.myMaterial.properties.blendequation = BlendEquation.subtract;
        ENSURE("obj.myMaterial.properties.blendequation == BlendEquation.subtract");
        ENSURE("obj.myMaterial.properties.blendequation == 'subtract'");
        switch (obj.myMaterial.properties.blendequation) {
            case BlendEquation.subtract:
                SUCCESS("switch");
                break;
            default:
                FAILURE("switch");
        }
        print(obj.myMaterial);
        ENSURE( obj.myMaterial.properties.targetbuffers.depth);
        obj.myMaterial.properties.targetbuffers.depth = false;
        ENSURE( ! obj.myMaterial.properties.targetbuffers.depth);


    }
};

function TestBitset() {
    this.Constructor(this, "TestBitset");
};
TestBitset.prototype.Constructor = function(obj, theName) {

    UnitTest.prototype.Constructor(obj, theName);

    obj.run = function() {
        obj.myBitset = new RenderStyles();
        ENSURE(' ! obj.myBitset.frontfacing');
        ENSURE(' ! obj.myBitset.backfacing');
        ENSURE(' ! obj.myBitset["backfacing"]');
        obj.myBitset.backfacing = true;
        obj.myBitset.frontfacing = true;
        ENSURE('obj.myBitset.backfacing');
        ENSURE('obj.myBitset["backfacing"]');

/*
        Nice to have:
        ENSURE_EXCEPTION('obj.myBitset.topfacing = true');
        obj.myBitset.setValue( RenderStyle.frontfacing, RenderStyle.backfacing, RenderStyle.ignore_depth);
        obj.myBitset.setValue( 0xff );
        var myMask = RenderStyles.asMask( RenderStyle.frontfacing, RenderStyle.backfacing, RenderStyle.ignore_depth);
        obj.myBitset.setValue( myMask );
*/
    }
};
var mySuite = new UnitTestSuite("UnitTest");

try {
    mySuite.addTest(new TestBitset);
    mySuite.addTest(new TestEnum);
    mySuite.run();
} catch (e) {
    print("## An unknown exception occured during execution." + e + "");
    mySuite.incrementFailedCount();
}

print(">> Finished test suite '"+mySuite.getMyName() +"', return status = " + mySuite.returnStatus() + "");
exit(mySuite.returnStatus());
