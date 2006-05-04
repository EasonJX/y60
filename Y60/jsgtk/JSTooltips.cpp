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
//   $RCSfile: JSTooltips.cpp,v $
//   $Author: david $
//   $Revision: 1.3 $
//   $Date: 2005/01/25 11:58:15 $
//
//
//=============================================================================

#include "JSTooltips.h"
#include "jsgtk.h"
#include <y60/JScppUtils.h>
#include <iostream>

using namespace std;
using namespace asl;

namespace jslib {

static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    std::string myStringRep = std::string("GtkTooltips@") + as_string(obj);
    *rval = as_jsval(cx, myStringRep);
    return JS_TRUE;
}

static JSBool
force_window(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    try {
        ensureParamCount(argc, 0);
        // native method call
        Gtk::Tooltips * myNative=0;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        myNative->force_window();
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
disable(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    try {
        ensureParamCount(argc, 0);
        // native method call
        Gtk::Tooltips * myNative=0;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        myNative->disable();
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
enable(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    try {
        ensureParamCount(argc, 0);
        // native method call
        Gtk::Tooltips * myNative=0;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        myNative->enable();
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
set_tip(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    try {
        ensureParamCount(argc, 2,3);
        Gtk::Widget * theWidget;
        Glib::ustring tipText;
        Glib::ustring privateText;
        
        if (!convertFrom(cx, argv[0], theWidget)) {
            JS_ReportError(cx, "JSTooltips::set_tip(): argument #1 must be a widget");
            return JS_FALSE;
        }
        if (!convertFrom(cx, argv[1], tipText)) {
            JS_ReportError(cx, "JSTooltips::set_tip(): argument #2 must be a string");
            return JS_FALSE;
        }
        if (argc ==3) {
            if (!convertFrom(cx, argv[2], privateText)) {
                JS_ReportError(cx, "JSTooltips::set_tip(): optional argument #3 must be a string.");
                return JS_FALSE;
            }
        }

        // native method call
        Gtk::Tooltips * myNative=0;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        if (argc==2) {
            myNative->set_tip(*theWidget, tipText);
        } else {
            myNative->set_tip(*theWidget, tipText, privateText);
        }
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
unset_tip(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    try {
        ensureParamCount(argc, 1);
        Gtk::Widget * theWidget;
        
        if (!convertFrom(cx, argv[0], theWidget)) {
            JS_ReportError(cx, "JSTooltips::unset_tip(): argument #1 must be a widget");
            return JS_FALSE;
        }
        
        // native method call
        Gtk::Tooltips * myNative=0;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
            myNative->unset_tip(*theWidget);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

JSFunctionSpec *
JSTooltips::Functions() {
    IF_REG(cerr << "Registering class '"<<ClassName()<<"'"<<endl);
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {"toString",             toString,                 0},
        {"force_window",         force_window,             0},
        {"disable",              disable,                  0},
        {"enable",               enable,                   0},
        {"set_tip",              set_tip,                  3},
        {"unset_tip",            unset_tip,                1},
        {0}
    };
    return myFunctions;
}


JSPropertySpec *
JSTooltips::Properties() {
    static JSPropertySpec myProperties[] = {
        {0}
    };
    return myProperties;
}
// getproperty handling
JSBool
JSTooltips::getPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case 0:
        default:
            JS_ReportError(cx,"JSTooltips::getProperty: index %d out of range", theID);
            return JS_FALSE;
    }
}

JSBool
JSTooltips::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return getPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

// setproperty handling
JSBool
JSTooltips::setPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case 0:
        default:
            JS_ReportError(cx,"JSTooltips::setProperty: index %d out of range", theID);
            return JS_FALSE;
    }
}

JSBool
JSTooltips::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return setPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSTooltips::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }

    NATIVE * newNative = 0;

    JSTooltips * myNewObject = 0;

    if (argc == 0) {
        newNative = new NATIVE();
        myNewObject = new JSTooltips(OWNERPTR(newNative), newNative);
    } else {
        JS_ReportError(cx,"Constructor for %s: bad number of arguments: expected none () %d",ClassName(), argc);
        return JS_FALSE;
    }

    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSTooltips::Constructor: bad parameters");
    return JS_FALSE;
}

void
JSTooltips::addClassProperties(JSContext * cx, JSObject * theClassProto) {
    JSA_AddFunctions(cx, theClassProto, Functions());
    JSA_AddProperties(cx, theClassProto, Properties());
    createClassModuleDocumentation("gtk", ClassName(), Properties(), Functions(),
            0, 0, 0, 0);
}

JSObject *
JSTooltips::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject * myClassObject = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, 0, 0);
    if (myClassObject) {
        addClassProperties(cx, myClassObject);
    }
    jsval myConstructorFuncObjVal;
    if (JS_GetProperty(cx, theGlobalObject, ClassName(), &myConstructorFuncObjVal)) {
        JSObject * myConstructorFuncObj = JSVAL_TO_OBJECT(myConstructorFuncObjVal);
    } else {
        cerr << "JSTooltips::initClass: constructor function object not found, could not initialize static members"<<endl;
    }
    return myClassObject;
}

jsval as_jsval(JSContext *cx, JSTooltips::OWNERPTR theOwner, JSTooltips::NATIVE * theNative) {
    JSObject * myReturnObject = JSTooltips::Construct(cx, theOwner, theNative);
    return OBJECT_TO_JSVAL(myReturnObject);
}

bool convertFrom(JSContext *cx, jsval theValue, JSTooltips::NATIVE *& theTarget) {
    if (JSVAL_IS_OBJECT(theValue)) {
        JSObject * myArgument;
        if (JS_ValueToObject(cx, theValue, &myArgument)) {
            if (JSA_GetClass(cx,myArgument) == JSClassTraits<JSTooltips::NATIVE>::Class()) {
                JSClassTraits<JSTooltips::NATIVE>::ScopedNativeRef myObj(cx, myArgument);
                theTarget = &myObj.getNative();
                return true;
            }
        }
    }
    return false;
}

}


