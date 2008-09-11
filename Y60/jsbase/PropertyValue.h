//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _PROPERTY_VALUE_INCLUDED
#define _PROPERTY_VALUE_INCLUDED

#include "JScppUtils.h"
#include <js/spidermonkey/jsapi.h>

namespace y60 {

class PropertyValue {
public:
    PropertyValue(JSContext *cx, jsval * theJSVal) :
        _myCx(cx), _myJsVal(theJSVal)
    {}

    template<class T>
    T get() const;

    template<class T>
    bool get(T & theReturnValue) const;

    template<class T>
    void set(const T & theValue); 
    
private:
    PropertyValue() {}
    PropertyValue(const PropertyValue &) {}
    PropertyValue & operator=(PropertyValue &) { return *this; }
    PropertyValue * operator&() { return this; }

    JSContext *_myCx;
    jsval * _myJsVal;
};

}

#endif
