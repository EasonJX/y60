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
//   $RCSfile: SDLFontInfo.h,v $
//   $Author: martin $
//   $Revision: 1.1 $
//   $Date: 2004/11/07 10:01:43 $
//
//  Description: A simple renderer.
//
//=============================================================================

#ifndef AC_Y60_SDLFONTINFO
#define AC_Y60_SDLFONTINFO

#include "y60_sdlengine_settings.h"

#include <y60/gltext/TTFFontInfo.h>

#include <sdl/ttf2/SDL_ttf.h>

namespace y60 {

    class SDLFontInfo : public TTFFontInfo {
        public:
            SDLFontInfo(TTF_Font* theFont, FONTTYPE theFontType, int theHeight, FONTHINTING theHinting )
                : TTFFontInfo(theFontType, theHeight, theHinting),
                _myFont(theFont)
            {}

            SDLFontInfo() {};

            TTF_Font * getFont() {
                return _myFont;
            }
            const TTF_Font * getFont() const {
                return _myFont;
            }
        private:
            TTF_Font * _myFont;
    };

} // namespace y60

#endif // AC_Y60_SDLFONTINFO
