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
//   $RCSfile: JSColorButton.cpp,v $
//   $Author: martin $
//   $Revision: 1.2 $
//   $Date: 2005/04/13 10:38:06 $
//
//
//=============================================================================

#include "JSColorButton.h"
#include "jsgtk.h"
#include <y60/JSVector.h>
#include <y60/JScppUtils.h>
#include <iostream>

using namespace std;
using namespace asl;

namespace jslib {


static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    std::string myStringRep = string("Gtk::ColorButton@") + as_string(obj);
    JSString * myString = JS_NewStringCopyN(cx,myStringRep.c_str(),myStringRep.size());
    *rval = STRING_TO_JSVAL(myString);
    return JS_TRUE;
}

JSFunctionSpec *
JSColorButton::Functions() {
    IF_REG(cerr << "Registering class '"<<ClassName()<<"'"<<endl);
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {"toString",             toString,                0},
        {0}
    };
    return myFunctions;
}

JSPropertySpec *
JSColorButton::Properties() {
    static JSPropertySpec myProperties[] = {
        {"color",     PROP_color,     JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"alpha",     PROP_alpha,     JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"use_alpha", PROP_use_alpha, JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"title",     PROP_title,     JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {0}
    };
    return myProperties;
}

// getproperty handling
JSBool
JSColorButton::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return getPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSColorButton::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return setPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSColorButton::getPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case PROP_color:
            {
                Gdk::Color myGdkColor = theNative.get_color();
                float myRed, myGreen, myBlue, myAlpha;
                myRed   = float(myGdkColor.get_red_p());
                myGreen = float(myGdkColor.get_green_p());
                myBlue  = float(myGdkColor.get_blue_p());
                if (theNative.get_use_alpha()) {
                    myAlpha = float(theNative.get_alpha() / 65535.0);
                } else {
                    myAlpha = 1.0f;
                }
                asl::Vector4f myColor(myRed, myGreen, myBlue, myAlpha);
                * vp = as_jsval(cx, myColor);

                return JS_TRUE;
            }
        case PROP_alpha:
            {
                guint16 myAlpha = theNative.get_alpha();
                * vp = as_jsval(cx, float( myAlpha / 65535.0));
                return JS_TRUE;
            }
        case PROP_use_alpha:
            {
                bool myUseAlphaFlag = theNative.get_use_alpha();
                * vp = as_jsval(cx, myUseAlphaFlag);
                return JS_TRUE;
            }
        case PROP_title:
            {
                Glib::ustring myTitle = theNative.get_title();
                * vp = as_jsval(cx, myTitle);
                return JS_TRUE;
            }
        default:
            return JSBASE::getPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}
JSBool
JSColorButton::setPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case PROP_color:
            {
                asl::Vector4f myColor;
                convertFrom(cx, * vp, myColor);
                Gdk::Color myGdkColor;
                myGdkColor.set_rgb_p(myColor[0], myColor[1], myColor[2]);
                theNative.set_color(myGdkColor);
                if (theNative.get_use_alpha()) {
                    theNative.set_alpha( guint16( 65535 * myColor[3]));
                }
                return JS_TRUE;
            }
        case PROP_alpha:
            {
                float myAlpha;
                convertFrom(cx, * vp, myAlpha);
                theNative.set_alpha( guint16( 65535 * myAlpha));
                return JS_TRUE;
            }
        case PROP_use_alpha:
            {
                bool myUseAlphaFlag;
                convertFrom(cx, * vp, myUseAlphaFlag);
                theNative.set_use_alpha(myUseAlphaFlag);
                return JS_TRUE;
            }
        case PROP_title:
            {
                Glib::ustring myTitle;
                convertFrom(cx, * vp, myTitle);
                theNative.set_title(myTitle);
                return JS_TRUE;
            }
        default:
            return JSBASE::setPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}

JSBool
JSColorButton::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }

    NATIVE * newNative = 0;

    JSColorButton * myNewObject = 0;

    if (argc == 0) {
        newNative = new Gtk::ColorButton();
        myNewObject = new JSColorButton(OWNERPTR(newNative), newNative);
    } else {
        JS_ReportError(cx,"Constructor for %s: bad number of arguments: expected none () %d",ClassName(), argc);
        return JS_FALSE;
    }

    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSColorButton::Constructor: bad parameters");
    return JS_FALSE;
}
/*
JSConstIntPropertySpec *
JSColorButton::ConstIntProperties() {

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
JSColorButton::addClassProperties(JSContext * cx, JSObject * theClassProto) {
    JSBASE::addClassProperties(cx, theClassProto);
    JSA_AddFunctions(cx, theClassProto, Functions());
    JSA_AddProperties(cx, theClassProto, Properties());
    createClassModuleDocumentation("gtk", ClassName(), Properties(), Functions(), 0, 0, 0,
            JSBASE::ClassName());
}

JSObject *
JSColorButton::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject * myClassObject = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, 0 ,0);
    if (myClassObject) {
        addClassProperties(cx, myClassObject);
    }
    jsval myConstructorFuncObjVal;
    if (JS_GetProperty(cx, theGlobalObject, ClassName(), &myConstructorFuncObjVal)) {
//        JSObject * myConstructorFuncObj = JSVAL_TO_OBJECT(myConstructorFuncObjVal);
//        JSA_DefineConstInts(cx, myConstructorFuncObj, ConstIntProperties());
    } else {
        cerr << "JSColorButton::initClass: constructor function object not found, could not initialize static members"<<endl;
    }
    return myClassObject;
}

jsval as_jsval(JSContext *cx, JSColorButton::OWNERPTR theOwner, JSColorButton::NATIVE * theNative) {
    JSObject * myReturnObject = JSColorButton::Construct(cx, theOwner, theNative);
    return OBJECT_TO_JSVAL(myReturnObject);
}

}




