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
*/
//
//    $RCSfile: TestTerrainTexGen.h,v $
//
//     $Author: uzadow $
//
//   $Revision: 1.1 $
//
//
// Description:
//
//=============================================================================

#if !defined(INCL_TESTFILTERTEXGEN)
#define INCL_TESTFILTERTEXGEN

#include <vector>

#include <asl/base/UnitTest.h>

#if defined(_MSC_VER)
#   pragma warning (push,1)
#endif //defined(_MSC_VER)
#include <paintlib/planybmp.h>
#if defined(_MSC_VER)
#   pragma warning (pop)
#endif //defined(_MSC_VER)

#include "TerrainTexGen.h"
#include "TextureDefinition.h"

class TestTerrainTexGen: public UnitTest {
public:
    explicit TestTerrainTexGen (): UnitTest ("TestTerrainTexGen") {}
    virtual void run();

private:
    TexGen::TextureDefinitionMap * createColorDefinitions ();
    TexGen::TextureDefinitionMap * createBmpDefinitions ();
    void runTestsWithOneSize
        (const TexGen::TerrainTexGen& theTerrainTexGen,
         PLBmp * theIndexBmp,
         const std::string& theResultFileName,
         bool runPartialTest,
         const PLPoint& theResultSize);
    void runTestsWithIndexBmp
        (float myBlendRadius,
         const std::string & theIndexFileName,
         const std::string & theBigResultFileName,
         const std::string & theSmallResultFileName,
         bool runPartialTest,
         bool useColorDefinition);

};


#endif

