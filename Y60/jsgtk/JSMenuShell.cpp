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
//   $RCSfile: JSMenuShell.cpp,v $
//   $Author: david $
//   $Revision: 1.2 $
//   $Date: 2005/04/13 15:24:13 $
//
//
//=============================================================================

#include "JSMenuShell.h"
#include "JSSignalProxy0.h"
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
    std::string myStringRep = string("Gtk::MenuShell@") + as_string(obj);
    *rval = as_jsval(cx, myStringRep);
    return JS_TRUE;
}

static JSBool
append(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    try {
        ensureParamCount(argc, 1);
        Gtk::MenuItem * myChild;
        if (!convertFrom(cx, argv[0], myChild)) {
            JS_ReportError(cx, "JSMenuShell::append(): First argument must be a widget.");
            return JS_FALSE;
        }
        Gtk::Menu * myNative(0);
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        myNative->append(*myChild);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
prepend(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    try {
        ensureParamCount(argc, 1);
        Gtk::MenuItem * myChild;
        if (!convertFrom(cx, argv[0], myChild)) {
            JS_ReportError(cx, "JSMenuShell::append(): First argument must be a widget.");
            return JS_FALSE;
        }
        Gtk::Menu * myNative(0);
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        myNative->prepend(*myChild);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
insert(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    try {
        ensureParamCount(argc, 2);
        Gtk::MenuItem * myChild;
        int myPosition;
        if (!convertFrom(cx, argv[0], myChild)) {
            JS_ReportError(cx, "JSMenuShell::append(): First argument must be a widget.");
            return JS_FALSE;
        }
        if (!convertFrom(cx, argv[1], myPosition)) {
            JS_ReportError(cx, "JSMenuShell::append(): Second argument must be a number.");
            return JS_FALSE;
        }
        Gtk::Menu * myNative(0);
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        myNative->insert(*myChild, myPosition);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}


JSFunctionSpec *
JSMenuShell::Functions() {
    IF_REG(cerr << "Registering class '"<<ClassName()<<"'"<<endl);
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {"toString",             toString,                0},
        {"append",               append,                  1},
        {"prepend",              prepend,                 1},
        {"insert",               insert,                  2},
        {0}
    };
    return myFunctions;
}
JSPropertySpec *
JSMenuShell::Properties() {
    static JSPropertySpec myProperties[] = {
        {"signal_deactivate", PROP_signal_deactivate, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"signal_selection_done", PROP_signal_selection_done, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {0}
    };
    return myProperties;
}

// property handling
JSBool
JSMenuShell::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return getPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSMenuShell::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return setPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSMenuShell::getPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case PROP_signal_deactivate:
            {
                JSSignalProxy0<void>::OWNERPTR mySignal( new
                        JSSignalProxy0<void>::NATIVE(theNative.signal_deactivate()));
                *vp = jslib::as_jsval(cx, mySignal);
                return JS_TRUE;
            }
        case PROP_signal_selection_done:
            {
                JSSignalProxy0<void>::OWNERPTR mySignal( new
                        JSSignalProxy0<void>::NATIVE(theNative.signal_selection_done()));
                *vp = jslib::as_jsval(cx, mySignal);
                return JS_TRUE;
            }
        default:
            return JSBASE::getPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}
JSBool
JSMenuShell::setPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case 0:
        default:
            return JSBASE::setPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}

JSBool
JSMenuShell::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }

    NATIVE * newNative = 0;

    JSMenuShell * myNewObject = 0;

    if (argc == 0) {
        newNative = 0; // Abstract
        myNewObject = new JSMenuShell(OWNERPTR(newNative), newNative);
    } else {
        JS_ReportError(cx,"Constructor for %s: bad number of arguments: expected none () %d",ClassName(), argc);
        return JS_FALSE;
    }

    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSMenuShell::Constructor: bad parameters");
    return JS_FALSE;
}

void
JSMenuShell::addClassProperties(JSContext * cx, JSObject * theClassProto) {
    JSBASE::addClassProperties(cx, theClassProto);
    JSA_AddProperties(cx, theClassProto, Properties());
    JSA_AddFunctions(cx, theClassProto, Functions());
    createClassModuleDocumentation("gtk", ClassName(), Properties(), Functions(), 0, 0, 0,
                JSBASE::ClassName());
}

JSObject *
JSMenuShell::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject * myClassObject = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, 0,0);
    if (myClassObject) {
        addClassProperties(cx, myClassObject);
    }
    jsval myConstructorFuncObjVal;
    if (JS_GetProperty(cx, theGlobalObject, ClassName(), &myConstructorFuncObjVal)) {
//        JSObject * myConstructorFuncObj = JSVAL_TO_OBJECT(myConstructorFuncObjVal);
//        JSA_DefineConstInts(cx, myConstructorFuncObj, ConstIntProperties());
    } else {
        cerr << "JSMenuShell::initClass: constructor function object not found, could not initialize static members"<<endl;
    }
    return myClassObject;
}

jsval as_jsval(JSContext *cx, JSMenuShell::OWNERPTR theOwner, JSMenuShell::NATIVE * theNative) {
    JSObject * myReturnObject = JSMenuShell::Construct(cx, theOwner, theNative);
    return OBJECT_TO_JSVAL(myReturnObject);
}

}

