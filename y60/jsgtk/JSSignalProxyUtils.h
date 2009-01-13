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
//   $RCSfile: JSSignalProxyUtils.h,v $
//   $Author: david $
//   $Revision: 1.1 $
//   $Date: 2005/03/03 17:28:53 $
//
//
//=============================================================================

#ifndef _Y60_ACGTKSHELL_JSSIGNAL_PROXY_UTILS_INCLUDED_
#define _Y60_ACGTKSHELL_JSSIGNAL_PROXY_UTILS_INCLUDED_

#include "y60_jsgtk_settings.h"

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4512)
#endif //defined(_MSC_VER)
#include <sigc++/connection.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif //defined(_MSC_VER)
#include <typeinfo>
#include <string>

template <class T>
struct TypeNameTrait {
    static const char * name() {
        return typeid(T()).name(); //"UnknownType";
    }
};

template <>
struct TypeNameTrait<void> {
    static const char * name() {
        return "void";
    }
};


struct JSObject;

namespace jslib {
    
class JSSignalAdapterBase {
    public:
        static void on_target_finalized(JSObject *, sigc::connection theConnection);
        static void on_target_finalized_dbg(JSObject *, sigc::connection theConnection, std::string theDebugMessage);
};

}

#endif
