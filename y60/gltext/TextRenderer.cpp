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

//own header
#include "TextRenderer.h"


#include <y60/glutil/GLUtils.h>

#include <asl/math/Vector234.h>
#include <asl/base/Dashboard.h>
#include <asl/math/numeric_functions.h>

#include <iostream>


using namespace std;
using namespace asl;

namespace y60 {

    TextRenderer::TextRenderer() :
        _myHorizontalAlignment(LEFT_ALIGNMENT),
        _myVerticalAlignment(TOP_ALIGNMENT),

        _myTopPadding(0),
        _myBottomPadding(0),
        _myLeftPadding(0),
        _myRightPadding(0),

        _myLineHeight(0),
        _myParagraphTopOffset(0),
        _myParagraphBottomOffset(0),
        _myTracking(0.0f),
        _myIndentation(0),
        _myCursorPos(0,0),
        _myCurrentTextColor(Vector4f(1.0, 1.0, 1.0, 1.0)),
        _myWindowWidth(0),
        _myWindowHeight(0)
    {}

    TextRenderer::~TextRenderer() {}

    TextPtr
    TextRenderer::createText(const Vector2f & thePos,
                             const string & theString,
                             const string & theFont)
    {
        return TextPtr(new Text(this, thePos, getTextColor(), theString, theFont));
    }


    void
    TextRenderer::setWindowSize(const unsigned int & theWindowWidth,
                               const unsigned int & theWindowHeight)
    {
        _myWindowWidth  = theWindowWidth;
        _myWindowHeight = theWindowHeight;
    }


    void
    TextRenderer::setColor(const asl::Vector4f & theTextColor) {
        _myCurrentTextColor = theTextColor;
    }
    const asl::Vector4f &
    TextRenderer::getColor() {
        return _myCurrentTextColor;
    }

    void
    TextRenderer::setPadding(int theTop, int theBottom, int theLeft, int theRight) {
        _myTopPadding = theTop;
        _myBottomPadding = theBottom;
        _myLeftPadding = theLeft;
        _myRightPadding = theRight;
    }

    void
    TextRenderer::setCursor(asl::Vector2i thePosition) {
        _myCursorPos = thePosition;
    }

    void
    TextRenderer::setIndentation(int theIndent) {
        _myIndentation = theIndent;
    }

	void
	TextRenderer::setVTextAlignment(TextRenderer::TextAligment theVAlignment) {
	    _myVerticalAlignment = theVAlignment;
	}

    void
    TextRenderer::setHTextAlignment(TextRenderer::TextAligment theHAlignment) {
        _myHorizontalAlignment = theHAlignment;
	}
}
