//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
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
