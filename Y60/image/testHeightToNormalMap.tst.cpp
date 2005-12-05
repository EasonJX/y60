/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2005, ART+COM AG Berlin, Germany
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
//    $RCSfile: testHeightToNormalMap.tst.cpp,v $
//
//   $Revision: 1.1 $
//
// Description: unit test template file - change Scene to whatever
//              you want to test and add the apprpriate tests.
//
//
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/

#include "HeightToNormalMap.h"

#include <asl/linearAlgebra.h>
#include <asl/numeric_functions.h>
#include <paintlib/plpaintlibdefs.h>

#include <asl/UnitTest.h>

#include <paintlib/plpixel8.h>
#include <paintlib/plpixel32.h>
#include <paintlib/planybmp.h>

#include <string>
#include <iostream>

using namespace std;  // automatically added!
using namespace asl;  // automatically added!

class HeightToNormalMapUnitTest : public UnitTest {
public:
    HeightToNormalMapUnitTest()
        : UnitTest("HeightToNormalMapUnitTest") {}

    void decompress(asl::Vector3f & v) {
        v = (v * 128.f);
        v.add(128.f);
        v[0] = asl::minimum(v[0], 255.f);
        v[1] = asl::minimum(v[1], 255.f);
        v[2] = asl::minimum(v[2], 255.f);
    }

    void run() {
        PLAnyBmp myHeightField;
        myHeightField.Create(3, 3, PLPixelFormat::I8);
        PLBYTE ** myRows = myHeightField.GetLineArray();
        *myRows[0] = 0; *(myRows[0] + 1) = 0;   *(myRows[0] + 2) = 0;
        *myRows[1] = 0; *(myRows[1] + 1) = 255; *(myRows[1] + 2) = 0;
        *myRows[2] = 0; *(myRows[2] + 1) = 255;   *(myRows[2] + 2) = 0;

        PLAnyBmp myNormalMap;
        HeightToNormalMap myFilter;
        myFilter.Apply( & myHeightField, & myNormalMap);

        PLPixel32 ** myNormals = (PLPixel32**)myNormalMap.GetLineArray();
        asl::Vector3f v11 = normalized(asl::Vector3f(0, 0, 1));
        asl::Vector3f v12 = normalized(asl::Vector3f(0, 1, 1));
        asl::Vector3f v13 = normalized(asl::Vector3f(0, 0, 1));

        asl::Vector3f v21 = normalized(asl::Vector3f(-1, 0, 1));
        asl::Vector3f v22 = normalized(asl::Vector3f(1, 0, 1));
        asl::Vector3f v23 = normalized(asl::Vector3f(0, 0, 1));

        asl::Vector3f v31 = normalized(asl::Vector3f(-1, 0, 1));
        asl::Vector3f v32 = normalized(asl::Vector3f(1, 0, 1));
        asl::Vector3f v33 = normalized(asl::Vector3f(0, 0, 1));

        decompress(v11);
        decompress(v12);
        decompress(v13);

        decompress(v21);
        decompress(v22);
        decompress(v23);

        decompress(v31);
        decompress(v32);
        decompress(v33);

        ENSURE(myNormals[0][0].GetR() == (PLBYTE)v11[0]);
        ENSURE(myNormals[0][0].GetG() == (PLBYTE)v11[1]);
        ENSURE(myNormals[0][0].GetB() == (PLBYTE)v11[2]);

        ENSURE(myNormals[0][1].GetR() == (PLBYTE)v12[0]);
        ENSURE(myNormals[0][1].GetG() == (PLBYTE)v12[1]);
        ENSURE(myNormals[0][1].GetB() == (PLBYTE)v12[2]);

        ENSURE(myNormals[0][2].GetR() == (PLBYTE)v13[0]);
        ENSURE(myNormals[0][2].GetG() == (PLBYTE)v13[1]);
        ENSURE(myNormals[0][2].GetB() == (PLBYTE)v13[2]);

        ENSURE(myNormals[1][0].GetR() == (PLBYTE)v21[0]);
        ENSURE(myNormals[1][0].GetG() == (PLBYTE)v21[1]);
        ENSURE(myNormals[1][0].GetB() == (PLBYTE)v21[2]);

        ENSURE(myNormals[1][1].GetR() == (PLBYTE)v22[0]);
        ENSURE(myNormals[1][1].GetG() == (PLBYTE)v22[1]);
        ENSURE(myNormals[1][1].GetB() == (PLBYTE)v22[2]);

        ENSURE(myNormals[1][2].GetR() == (PLBYTE)v23[0]);
        ENSURE(myNormals[1][2].GetG() == (PLBYTE)v23[1]);
        ENSURE(myNormals[1][2].GetB() == (PLBYTE)v23[2]);

        ENSURE(myNormals[2][0].GetR() == (PLBYTE)v31[0]);
        ENSURE(myNormals[2][0].GetG() == (PLBYTE)v31[1]);
        ENSURE(myNormals[2][0].GetB() == (PLBYTE)v31[2]);

        ENSURE(myNormals[2][1].GetR() == (PLBYTE)v32[0]);
        ENSURE(myNormals[2][1].GetG() == (PLBYTE)v32[1]);
        ENSURE(myNormals[2][1].GetB() == (PLBYTE)v32[2]);

        ENSURE(myNormals[2][2].GetR() == (PLBYTE)v33[0]);
        ENSURE(myNormals[2][2].GetG() == (PLBYTE)v33[1]);
        ENSURE(myNormals[2][2].GetB() == (PLBYTE)v33[2]);

    }

};


class MyTestSuite : public UnitTestSuite {
public:
    MyTestSuite(const char * myName, int argc, char *argv[]) : UnitTestSuite(myName, argc, argv) {}
    void setup() {
        UnitTestSuite::setup(); // called to print a launch message
        addTest(new HeightToNormalMapUnitTest());
    }
};


int main(int argc, char *argv[]) {

    MyTestSuite mySuite(argv[0], argc, argv);

    mySuite.run();

    cerr << ">> Finished test suite '" << argv[0] << "'"
         << ", return status = " << mySuite.returnStatus() << endl;

    return mySuite.returnStatus();

}
