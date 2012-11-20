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

#ifndef __Y60_ACXPSHELL_GTKCPP_INCLUDED__
#define __Y60_ACXPSHELL_GTKCPP_INCLUDED__

#include "y60_jsgtk_settings.h"

#include "JSgtkutils.h"

#include <y60/jsbase/jssettings.h>

#ifdef USE_LEGACY_SPIDERMONKEY
#include <js/spidermonkey/jsapi.h>
#else
#include <js/jsapi.h>
#endif

#if defined(_MSC_VER)
    #pragma warning(push,1)
    //#pragma warning(disable:4512 4413 4250)
#endif //defined(_MSC_VER)
#include <gtkmm.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif //defined(_MSC_VER)

extern bool initGtkClasses(JSContext *cx, JSObject *theGlobalObject);

namespace jslib {

template<class TARGET>
class ConvertFrom {
    public:
        static bool convert(JSContext *cx, jsval theValue, Glib::RefPtr<TARGET> & theTarget);
        static bool convert(JSContext *cx, jsval theValue, TARGET *& theTarget);
};

template<class TARGET>
bool convertFrom(JSContext *cx, jsval theValue, TARGET *& theTarget) {
    return ConvertFrom<TARGET>::convert(cx, theValue, theTarget);
}

template<class TARGET>
bool convertFrom(JSContext *cx, jsval theValue, Glib::RefPtr<TARGET> & theTarget) {
    return ConvertFrom<TARGET>::convert(cx, theValue, theTarget);
}

Y60_JSGTK_DECL jsval gtk_jsval(JSContext *cx, Gtk::Widget * theWidget, bool takeOwnership = false);


} // namespace

#endif
