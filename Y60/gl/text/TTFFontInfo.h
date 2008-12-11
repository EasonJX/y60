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
//
//   $RCSfile: TTFFontInfo.h,v $
//   $Author: martin $
//   $Revision: 1.1 $
//   $Date: 2004/11/07 10:01:43 $
//
//  Description: A simple renderer.
//
//=============================================================================

#ifndef AC_Y60_TTFFONTINFO
#define AC_Y60_TTFFONTINFO

namespace y60 {

    static const char * FontTypeStrings[] = {
        "Normal",
        "Bold",
        "Italic",
        "BoldItalic",
        0
    };
    
    static const char * FontHintStrings[] = {
        "NoHinting",
        "NativeHinting",
        "AutoHinting",
        0
    };

    class TTFFontInfo {
        public:
            enum FONTTYPE {
                NORMAL,
                BOLD,
                ITALIC,
                BOLDITALIC
            };

            enum FONTHINTING {
                NOHINTING,
                NATIVEHINTING,
                AUTOHINTING
            };

            TTFFontInfo(FONTTYPE theFontType, int theHeight, FONTHINTING theHinting) :
                _myFontType(theFontType), _myHeight(theHeight), 
                _myFontHinting(theHinting)  {}

            TTFFontInfo() : _myFontType(NORMAL),_myFontHinting(NOHINTING) {};

            const FONTTYPE getFontType() {
                return _myFontType;
            }
            const FONTHINTING getFontHinting() {
                return _myFontHinting;
            }
            int getHeight() {
                return _myHeight;
            }
        private:
            FONTTYPE   _myFontType;
            FONTHINTING _myFontHinting;
            int        _myHeight;
    };

} // namespace y60

#endif 

