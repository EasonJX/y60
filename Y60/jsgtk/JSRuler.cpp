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
//   $RCSfile: JSRuler.cpp,v $
//   $Author: martin $
//   $Revision: 1.3 $
//   $Date: 2005/04/21 16:25:02 $
//
//
//=============================================================================

#include "JSRuler.h"
#include "jsgtk.h"
#include <y60/JScppUtils.h>
#include <iostream>

using namespace std;
using namespace asl;

namespace jslib {

static JSBool
Set_Range(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    try {
        ensureParamCount(argc, 4);
        // native method call
        Gtk::Ruler * myNative=0;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);

        float theLower;
        float theUpper;
        float thePosition;
        float theMaxSize;

        convertFrom(cx, argv[0], theLower);
        convertFrom(cx, argv[1], theUpper);
        convertFrom(cx, argv[2], thePosition);
        convertFrom(cx, argv[3], theMaxSize);
        myNative->set_range(theLower, theUpper, thePosition, theMaxSize);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    std::string myStringRep = string("Gtk::Ruler@") + as_string(obj);
    JSString * myString = JS_NewStringCopyN(cx,myStringRep.c_str(),myStringRep.size());
    *rval = STRING_TO_JSVAL(myString);
    return JS_TRUE;
}

JSFunctionSpec *
JSRuler::Functions() {
    IF_REG(cerr << "Registering class '"<<ClassName()<<"'"<<endl);
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {"toString",             toString,                0},
        {"set_range",            Set_Range,               4},
        {0}
    };
    return myFunctions;
}

JSPropertySpec *
JSRuler::Properties() {
    static JSPropertySpec myProperties[] = {
        {"metric", PROP_metric, JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"upper",  PROP_upper, JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"lower",  PROP_lower, JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"position", PROP_position, JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"max_size", PROP_max_size, JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {0}
    };
    return myProperties;
}

// property handling
JSBool
JSRuler::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return getPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSRuler::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return setPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSRuler::getPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    double myUpper, myLower, myPosition, myMaxSize;
    theNative.get_range(myLower, myUpper, myPosition, myMaxSize);
    switch (theID) {
        case PROP_metric:
            *vp = as_jsval(cx, static_cast<int>(theNative.get_metric()));
            return JS_TRUE;
        case PROP_upper:
            *vp = as_jsval(cx, myUpper);
            return JS_TRUE;
        case PROP_lower:
            *vp = as_jsval(cx, myLower);
            return JS_TRUE;
        case PROP_position:
            *vp = as_jsval(cx, myPosition);
            return JS_TRUE;
        case PROP_max_size:
            *vp = as_jsval(cx, myMaxSize);
            return JS_TRUE;
        default:
            return JSBASE::getPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}
JSBool
JSRuler::setPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    double myUpper, myLower, myPosition, myMaxSize;
    theNative.get_range(myLower, myUpper, myPosition, myMaxSize);
    switch (theID) {
        case PROP_metric:
            try {
                int theMetric;
                convertFrom(cx, *vp, theMetric);
                theNative.set_metric(static_cast<Gtk::MetricType>(theMetric));
                return JS_TRUE;
            } HANDLE_CPP_EXCEPTION;
        case PROP_upper:
            try {
                convertFrom(cx, *vp, myUpper);
                theNative.set_range(myLower, myUpper, myPosition, myMaxSize);
                return JS_TRUE;
            } HANDLE_CPP_EXCEPTION;
        case PROP_lower:
            try {
                convertFrom(cx, *vp, myLower);
                theNative.set_range(myLower, myUpper, myPosition, myMaxSize);
                return JS_TRUE;
            } HANDLE_CPP_EXCEPTION;
        case PROP_position:
            try {
                convertFrom(cx, *vp, myPosition);
                theNative.set_range(myLower, myUpper, myPosition, myMaxSize);
                return JS_TRUE;
            } HANDLE_CPP_EXCEPTION;
        case PROP_max_size:
            try {
                convertFrom(cx, *vp, myMaxSize);
                theNative.set_range(myLower, myUpper, myPosition, myMaxSize);
                return JS_TRUE;
            } HANDLE_CPP_EXCEPTION;
        default:
            return JSBASE::setPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}

JSBool
JSRuler::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }

    NATIVE * newNative = 0;

    JSRuler * myNewObject = 0;

    if (argc == 0) {
        newNative = 0; // Abstract
        myNewObject = new JSRuler(OWNERPTR(newNative), newNative);
    } else {
        JS_ReportError(cx,"Constructor for %s: bad number of arguments: expected none () %d",ClassName(), argc);
        return JS_FALSE;
    }

    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSRuler::Constructor: bad parameters");
    return JS_FALSE;
}

#define DEFINE_PROP( NAME ) { #NAME, PROP_ ## NAME, Gtk::NAME}

JSConstIntPropertySpec *
JSRuler::ConstIntProperties() {
    static JSConstIntPropertySpec myProperties[] = {
        DEFINE_PROP(PIXELS),
        DEFINE_PROP(INCHES),
        DEFINE_PROP(CENTIMETERS),
       {0}
    };
    return myProperties;
};

void
JSRuler::addClassProperties(JSContext * cx, JSObject * theClassProto) {
    JSBASE::addClassProperties(cx, theClassProto);
    JSA_AddProperties(cx, theClassProto, Properties());
    JSA_AddFunctions(cx, theClassProto, Functions());
    createClassModuleDocumentation("gtk", ClassName(), Properties(), Functions(),
            ConstIntProperties(), 0, 0, JSBASE::ClassName());
}

JSObject *
JSRuler::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject * myClassObject = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, 0, 0);
    if (myClassObject) {
        addClassProperties(cx, myClassObject);
    }
    jsval myConstructorFuncObjVal;
    if (JS_GetProperty(cx, theGlobalObject, ClassName(), &myConstructorFuncObjVal)) {
        JSObject * myConstructorFuncObj = JSVAL_TO_OBJECT(myConstructorFuncObjVal);
        JSA_DefineConstInts(cx, myConstructorFuncObj, ConstIntProperties());
    } else {
        cerr << "JSRuler::initClass: constructor function object not found, could not initialize static members"<<endl;
    }
    return myClassObject;
}

jsval as_jsval(JSContext *cx, JSRuler::OWNERPTR theOwner, JSRuler::NATIVE * theNative) {
    JSObject * myReturnObject = JSRuler::Construct(cx, theOwner, theNative);
    return OBJECT_TO_JSVAL(myReturnObject);
}

}

