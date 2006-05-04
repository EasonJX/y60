//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "JSACIconFactory.h"
#include "jsgtk.h"
#include <y60/JScppUtils.h>
#include <iostream>

using namespace std;
using namespace asl;

namespace jslib {

typedef JSACIconFactory::NATIVE NATIVE;

static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    std::string myStringRep = string("ACIconFactory@") + as_string(obj);
    *rval = as_jsval(cx, myStringRep);
    return JS_TRUE;
}

static JSBool
add(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;

    try {
        JSACIconFactory::OWNERPTR myNative;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);

        Glib::ustring myStockId;
        if ( ! convertFrom(cx, argv[0], myStockId)) {
            JS_ReportError(cx, "ACIconFactory::add() argument zero must be a string.");
            return JS_FALSE;
        }

        Glib::ustring myIconPath;
        if ( ! convertFrom(cx, argv[1], myIconPath)) {
            JS_ReportError(cx, "ACIconFactory::add() argument one must be a string.");
            return JS_FALSE;
        }

        Glib::ustring myLabel;
        if ( ! convertFrom(cx, argv[2], myLabel)) {
            JS_ReportError(cx, "ACIconFactory::add() argument two must be a string.");
            return JS_FALSE;
        }

        std::string myIconWithPath = searchFileRelativeToJSInclude(cx, obj, argc, argv, myIconPath);
        myNative->add(myStockId, myIconWithPath, myLabel);
        return JS_TRUE;

    } HANDLE_CPP_EXCEPTION;
}


JSFunctionSpec *
JSACIconFactory::Functions() {
    AC_DEBUG << "Registering class '"<<ClassName()<<"'"<<endl;
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {"toString",             toString,                0},
        {"add",                  add,                     2},
        {0}
    };
    return myFunctions;
}

JSPropertySpec *
JSACIconFactory::Properties() {
    static JSPropertySpec myProperties[] = {
        {0}
    };
    return myProperties;
}

JSConstIntPropertySpec *
JSACIconFactory::ConstIntProperties() {
    static JSConstIntPropertySpec myProperties[] = {{0}};
    return myProperties;
}

JSPropertySpec *
JSACIconFactory::StaticProperties() {
    static JSPropertySpec myProperties[] = {{0}};
    return myProperties;
}

JSFunctionSpec *
JSACIconFactory::StaticFunctions() {
    static JSFunctionSpec myFunctions[] = {{0}};
    return myFunctions;
}

// getproperty handling
JSBool
JSACIconFactory::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
            case 0:
            default:
                JS_ReportError(cx,"JSACIconFactory::getProperty: index %d out of range", theID);
                return JS_FALSE;
    }
}

// setproperty handling
JSBool
JSACIconFactory::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
        case 0:
        default:
            JS_ReportError(cx,"JSACIconFactory::setPropertySwitch: index %d out of range", theID);
            return JS_FALSE;
    }
}

JSBool
JSACIconFactory::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }
    JSACIconFactory * myNewObject = 0;

    OWNERPTR myNewACIconFactory = OWNERPTR(new NATIVE());

    myNewObject = new JSACIconFactory(myNewACIconFactory, &(*myNewACIconFactory));
    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSACIconFactory::Constructor: bad parameters");
    return JS_FALSE;
}

JSObject *
JSACIconFactory::initClass(JSContext *cx, JSObject *theGlobalObject) {
    DOC_CREATE(JSACIconFactory);
    return Base::initClass(cx, theGlobalObject, ClassName(), Constructor, Properties(), Functions());
}

bool convertFrom(JSContext *cx, jsval theValue, JSACIconFactory::OWNERPTR & theACIconFactory) {
    if (JSVAL_IS_OBJECT(theValue)) {
        JSObject * myArgument;
        if (JS_ValueToObject(cx, theValue, &myArgument)) {
            if (JSA_GetClass(cx,myArgument) == JSClassTraits<JSACIconFactory::NATIVE >::Class()) {
                theACIconFactory = JSClassTraits<JSACIconFactory::NATIVE>::getNativeOwner(cx,myArgument);
                return true;
            }
        }
    }
    return false;
}

jsval as_jsval(JSContext *cx, JSACIconFactory::OWNERPTR theOwner) {
    JSObject * myReturnObject = JSACIconFactory::Construct(cx, theOwner, &(*theOwner));
    return OBJECT_TO_JSVAL(myReturnObject);
}

/*
jsval as_jsval(JSContext *cx, JSACIconFactory::OWNERPTR theOwner, JSACIconFactory::NATIVE * theSerial) {
    JSObject * myObject = JSACIconFactory::Construct(cx, theOwner, theSerial);
    return OBJECT_TO_JSVAL(myObject);
}
*/

}

