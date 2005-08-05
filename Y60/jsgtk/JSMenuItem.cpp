//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: JSMenuItem.cpp,v $
//   $Author: david $
//   $Revision: 1.2 $
//   $Date: 2005/04/13 15:24:13 $
//
//
//=============================================================================

#include "JSMenuItem.h"
#include "JSBin.h"
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
    std::string myStringRep = string("Gtk::Bin@") + as_string(obj);
    JSString * myString = JS_NewStringCopyN(cx,myStringRep.c_str(),myStringRep.size());
    *rval = STRING_TO_JSVAL(myString);
    return JS_TRUE;
}

JSFunctionSpec *
JSMenuItem::Functions() {
    IF_REG(cerr << "Registering class '"<<ClassName()<<"'"<<endl);
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {"toString",             toString,                0},
        {0}
    };
    return myFunctions;
}

JSPropertySpec *
JSMenuItem::Properties() {
    static JSPropertySpec myProperties[] = {
        {"signal_activate", PROP_signal_activate, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"signal_activate_item", PROP_signal_activate_item, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"submenu", PROP_submenu, JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {0}
    };
    return myProperties;
}

// property handling
JSBool
JSMenuItem::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return getPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSMenuItem::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return setPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSMenuItem::getPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case PROP_signal_activate:
            {
                JSSignalProxy0<void>::OWNERPTR mySignal( new
                        JSSignalProxy0<void>::NATIVE(theNative.signal_activate()));
                *vp = jslib::as_jsval(cx, mySignal);
                return JS_TRUE;
            }
        case PROP_signal_activate_item:
            {
                JSSignalProxy0<void>::OWNERPTR mySignal( new
                        JSSignalProxy0<void>::NATIVE(theNative.signal_activate_item()));
                *vp = jslib::as_jsval(cx, mySignal);
                return JS_TRUE;
            }
        case PROP_submenu:
            {
                Gtk::Menu * mySubmenu = theNative.get_submenu();                
                *vp = jslib::as_jsval(cx, mySubmenu);
                return JS_TRUE;
            }
        default:
            return JSBASE::getPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}
JSBool
JSMenuItem::setPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case PROP_submenu:
            {
                Gtk::Menu * mySubMenu;
                convertFrom(cx, *vp, mySubMenu);
                if (mySubMenu) {
                    theNative.set_submenu(*mySubMenu);
                } else {
                    theNative.remove_submenu();
                }
                return JS_TRUE;
            }
        default:
            return JSBASE::setPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}

JSBool
JSMenuItem::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }

    NATIVE * newNative = 0;

    JSMenuItem * myNewObject = 0;

    if (argc == 0) {
        newNative = new NATIVE(); // Abstract        
    } else if (argc == 1) {
        Gtk::Widget * myWidget = 0;

        // argument must be a widget or a string
        if (!convertFrom(cx, argv[0], myWidget)) {
            Glib::ustring myString;
            if (!convertFrom(cx, argv[0], myString)) {
                JS_ReportError(cx, "JSMenuItem::JSMenuItem(): Argument must be a widget or a string.");
                return JS_FALSE;
            } else {
                newNative = new NATIVE(myString);
            }
        } else {
            newNative = new NATIVE(*myWidget);
        }
    } else if (argc == 2) {
        Glib::ustring myString;
        bool myMnemonicFlag;

        if (!convertFrom(cx, argv[0], myString)) {
            JS_ReportError(cx, "JSMenuItem::JSMenuItem(): First argument must be a string.");
            return JS_FALSE;
        }
        if (!convertFrom(cx, argv[1], myMnemonicFlag)) {
            JS_ReportError(cx, "JSMenuItem::JSMenuItem(): Second argument must be a bool.");
            return JS_FALSE;
        }
    } else {
        JS_ReportError(cx,"Constructor for %s: bad number of arguments: expected none () %d",ClassName(), argc);
        return JS_FALSE;
    }
    myNewObject = new JSMenuItem(OWNERPTR(newNative), newNative);
    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSMenuItem::Constructor: bad parameters");
    return JS_FALSE;
}
/*
JSConstIntPropertySpec *
JSBin::ConstIntProperties() {

    static JSConstIntPropertySpec myProperties[] = {
            "POST",              PROP_HTTP_POST,         Request::HTTP_POST,
            "GET",               PROP_HTTP_GET,          Request::HTTP_GET,
            "PUT",               PROP_HTTP_PUT,          Request::HTTP_PUT,
        {0}
    };
    return myProperties;
};
*/

void
JSMenuItem::addClassProperties(JSContext * cx, JSObject * theClassProto) {
    JSBASE::addClassProperties(cx, theClassProto);
    JSA_AddProperties(cx, theClassProto, Properties());
    JSA_AddFunctions(cx, theClassProto, Functions());
    createClassModuleDocumentation("gtk", ClassName(), Properties(), Functions(), 0, 0, 0,
                JSBASE::ClassName());
}

JSObject *
JSMenuItem::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject * myClassObject = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, 0,0);
    if (myClassObject) {
        addClassProperties(cx, myClassObject);
    }
    jsval myConstructorFuncObjVal;
    if (JS_GetProperty(cx, theGlobalObject, ClassName(), &myConstructorFuncObjVal)) {
//        JSObject * myConstructorFuncObj = JSVAL_TO_OBJECT(myConstructorFuncObjVal);
//        JSA_DefineConstInts(cx, myConstructorFuncObj, ConstIntProperties());
    } else {
        cerr << "JSMenuItem::initClass: constructor function object not found, could not initialize static members"<<endl;
    }
    return myClassObject;
}

jsval as_jsval(JSContext *cx, JSMenuItem::OWNERPTR theOwner, JSMenuItem::NATIVE * theNative) {
    JSObject * myReturnObject = JSMenuItem::Construct(cx, theOwner, theNative);
    return OBJECT_TO_JSVAL(myReturnObject);
}

}
