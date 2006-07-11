//=============================================================================
// Copyright (C) 2003-2004, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "JSCMSCache.h"
#include <y60/JSNode.h>


#include <asl/PackageManager.h>

#include <iostream>

using namespace std;
using namespace y60;

#define DB(x) // x

namespace jslib {

static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    *rval = as_jsval(cx, "CMSCache");
    return JS_TRUE;
}

static JSBool
synchronize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    return Method<JSCMSCache::NATIVE>::call(&JSCMSCache::NATIVE::synchronize,cx,obj,argc,argv,rval);
}

static JSBool
isSynchronized(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    return Method<JSCMSCache::NATIVE>::call(&JSCMSCache::NATIVE::isSynchronized,cx,obj,argc,argv,rval);
}

#define DEFINE_ORIENTATION_PROP(NAME) { #NAME, PROP_ ## NAME , CMSCache::NAME }

JSConstIntPropertySpec *
JSCMSCache::ConstIntProperties() {
    static JSConstIntPropertySpec myProperties[] = {
        {0}
    };
    return myProperties;
};

JSFunctionSpec *
JSCMSCache::Functions() {
    AC_DEBUG << "Registering class '"<<ClassName()<<"'"<<endl;
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {"synchronize",     synchronize,                0},
        {"isSynchronized",     isSynchronized,            0},
        {0}
    };
    return myFunctions;
}

JSFunctionSpec *
JSCMSCache::StaticFunctions() {
    IF_REG(cerr << "Registering class '"<<ClassName()<<"'"<<endl);
    static JSFunctionSpec myFunctions[] = {
        {0}
    };
    return myFunctions;
}


JSPropertySpec *
JSCMSCache::Properties() {
    static JSPropertySpec myProperties[] = {
        {"statusReport", PROP_statusReport,
                JSPROP_ENUMERATE | JSPROP_PERMANENT | JSPROP_READONLY},
        {"verbose", PROP_verbose, JSPROP_ENUMERATE | JSPROP_PERMANENT},
        {0}
    };
    return myProperties;
}

// getproperty handling
JSBool
JSCMSCache::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj); 
    switch (theID) {
        case PROP_statusReport:
            *vp = as_jsval( cx, static_cast_Ptr<dom::Node>( myObj.getNative().getStatusReport()));
            return JS_TRUE;
        case PROP_verbose:
            *vp = as_jsval( cx,  myObj.getNative().getVerboseFlag());
            return JS_TRUE;
        default:
            JS_ReportError(cx,"JSCMSCache::getProperty: index %d out of range", theID);
            return JS_FALSE;
    }
}

// setproperty handling
JSBool
JSCMSCache::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj); 
    switch (theID) {
        case PROP_verbose:
            try {
                bool myVerboseFlag;
                convertFrom(cx, *vp, myVerboseFlag );
                myObj.getNative().setVerboseFlag( myVerboseFlag );
                return JS_TRUE;
            } HANDLE_CPP_EXCEPTION;
        default:
            JS_ReportError(cx,"JSCMSCache::setPropertySwitch: index %d out of range", theID);
            return JS_FALSE;
    }
}

JSBool
JSCMSCache::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s bad object; did you forget a 'new'?", ClassName());
        return JS_FALSE;
    }

    OWNERPTR myNewNative;
    
    std::string myUsername;
    std::string myPassword;
    std::string myLocalPath;
    dom::NodePtr myPresentationDoc;
        
    switch (argc) {
        case 4:
            convertFrom(cx, argv[2], myUsername);
            convertFrom(cx, argv[3], myPassword);
            // XXX don't break here
        case 2:
            convertFrom(cx, argv[0], myLocalPath);
            convertFrom(cx, argv[1], myPresentationDoc);
            break;
        default:
            JS_ReportError(cx, "Constructor for %s: bad number of arguments: expected three or five "
                    ", got %d", ClassName(), argc);
            return JS_FALSE;
    }
    
    switch (argc) {
        case 2:
            myNewNative = OWNERPTR(new CMSCache(myLocalPath, myPresentationDoc));
            break;    
        case 4:
            myNewNative = OWNERPTR(new CMSCache(myLocalPath, myPresentationDoc,
                                                  myUsername, myPassword));
            break;
    }

    JSCMSCache * myNewObject = new JSCMSCache(myNewNative, &(*myNewNative));
    JS_SetPrivate(cx, obj, myNewObject);
    return JS_TRUE;
}

JSObject *
JSCMSCache::initClass(JSContext *cx, JSObject *theGlobalObject) {
    return Base::initClass(cx, theGlobalObject, ClassName(), Constructor, Properties(), Functions(), ConstIntProperties(), 0, StaticFunctions());
}

bool convertFrom(JSContext *cx, jsval theValue, JSCMSCache::OWNERPTR & theCMSCache) {
    if (JSVAL_IS_OBJECT(theValue)) {
        JSObject * myArgument;
        if (JS_ValueToObject(cx, theValue, &myArgument)) {
            if (JSA_GetClass(cx,myArgument) == JSClassTraits<JSCMSCache::NATIVE >::Class()) {
                theCMSCache = JSClassTraits<JSCMSCache::NATIVE>::getNativeOwner(cx,myArgument);
                return true;
            }
        }
    }
    return false;
}

jsval as_jsval(JSContext *cx, JSCMSCache::OWNERPTR theOwner) {
    JSObject * myReturnObject = JSCMSCache::Construct(cx, theOwner, &(*theOwner));
    return OBJECT_TO_JSVAL(myReturnObject);
}

jsval as_jsval(JSContext *cx, JSCMSCache::OWNERPTR theOwner, JSCMSCache::NATIVE * theCMSCache) {
    JSObject * myObject = JSCMSCache::Construct(cx, theOwner, theCMSCache);
    return OBJECT_TO_JSVAL(myObject);
}

}
