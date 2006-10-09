//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================


#include "JSSignalProxies.h"
#include "JSColorSelection.h"
#include "JSBox.h"
#include "jsgtk.h"
#include <y60/JScppUtils.h>
#include <y60/JSVector.h>
#include <iostream>

using namespace std;
using namespace asl;

namespace jslib {


static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    std::string myStringRep = string("Gtk::ColorSelection@") + as_string(obj);
    *rval = as_jsval(cx, myStringRep);
    return JS_TRUE;
}

JSFunctionSpec *
JSColorSelection::Functions() {
    IF_REG(cerr << "Registering class '"<<ClassName()<<"'"<<endl);
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {"toString",             toString,                0},
        {0}
    };
    return myFunctions;
}

#define DEFINE_PROPERTY( NAME ) \
    { #NAME, PROP_ ## NAME,  JSPROP_ENUMERATE|JSPROP_PERMANENT}

#define DEFINE_RO_PROPERTY( NAME ) \
    { #NAME, PROP_ ## NAME,  JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT}
JSPropertySpec *
JSColorSelection::Properties() {
    static JSPropertySpec myProperties[] = {
        DEFINE_PROPERTY( current_color ),
        DEFINE_PROPERTY( previous_color ),
        DEFINE_PROPERTY( has_opacity_control ),
        DEFINE_PROPERTY( has_palette ),
        DEFINE_RO_PROPERTY( signal_color_changed ),
        {0}
    };
    return myProperties;
}

// property handling
JSBool
JSColorSelection::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return getPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSColorSelection::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return setPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSColorSelection::getPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case PROP_current_color:
            {
                Gdk::Color myGdkColor = theNative.get_current_color();
                float myRed, myGreen, myBlue, myAlpha;
                myRed   = float(myGdkColor.get_red_p());
                myGreen = float(myGdkColor.get_green_p());
                myBlue  = float(myGdkColor.get_blue_p());
                if (theNative.get_has_opacity_control()) {
                    myAlpha = float(theNative.get_current_alpha() / 65535.0);
                } else {
                    myAlpha = 1.0f;
                }
                asl::Vector4f myColor(myRed, myGreen, myBlue, myAlpha);
                * vp = as_jsval(cx, myColor);

                return JS_TRUE;
            }
        case PROP_previous_color:
            {
                Gdk::Color myGdkColor = theNative.get_previous_color();
                float myRed, myGreen, myBlue, myAlpha;
                myRed   = float(myGdkColor.get_red_p());
                myGreen = float(myGdkColor.get_green_p());
                myBlue  = float(myGdkColor.get_blue_p());
                if (theNative.get_has_opacity_control()) {
                    myAlpha = float(theNative.get_previous_alpha() / 65535.0);
                } else {
                    myAlpha = 1.0f;
                }
                asl::Vector4f myColor(myRed, myGreen, myBlue, myAlpha);
                * vp = as_jsval(cx, myColor);

                return JS_TRUE;
            }
            return JS_TRUE;
        case PROP_has_opacity_control:
            *vp = as_jsval(cx, theNative.get_has_opacity_control());
            return JS_TRUE;
        case PROP_has_palette:
            *vp = as_jsval(cx, theNative.get_has_palette());
            return JS_TRUE;
        case PROP_signal_color_changed:
            {
                JSSignalProxy0<void>::OWNERPTR mySignal( new
                        JSSignalProxy0<void>::NATIVE(theNative.signal_color_changed()));
                *vp = jslib::as_jsval(cx, mySignal);
                return JS_TRUE;
            }
        default:
            return JSBASE::getPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}
JSBool
JSColorSelection::setPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case PROP_current_color:
            {
                asl::Vector4f myColor;
                convertFrom(cx, * vp, myColor);
                Gdk::Color myGdkColor;
                myGdkColor.set_rgb_p(myColor[0], myColor[1], myColor[2]);
                theNative.set_current_color(myGdkColor);
                if (theNative.get_has_opacity_control()) {
                    theNative.set_current_alpha( guint16( 65535 * myColor[3]));
                }
                return JS_TRUE;
            }
        case PROP_previous_color:
            {
                asl::Vector4f myColor;
                convertFrom(cx, * vp, myColor);
                Gdk::Color myGdkColor;
                myGdkColor.set_rgb_p(myColor[0], myColor[1], myColor[2]);
                theNative.set_previous_color(myGdkColor);
                if (theNative.get_has_opacity_control()) {
                    theNative.set_previous_alpha( guint16( 65535 * myColor[3]));
                }
                return JS_TRUE;
            }
        case PROP_has_opacity_control:
            {
                bool myHasOpacityFlag;
                convertFrom(cx, * vp, myHasOpacityFlag);
                theNative.set_has_opacity_control(myHasOpacityFlag);
                return JS_TRUE;
            }
        case PROP_has_palette:
            {
                bool myHasPaletteFlag;
                convertFrom(cx, * vp, myHasPaletteFlag);
                theNative.set_has_palette(myHasPaletteFlag);
                return JS_TRUE;
            }
            return JS_TRUE;
        default:
            return JSBASE::setPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}

JSBool
JSColorSelection::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }

    NATIVE * newNative = 0;

    JSColorSelection * myNewObject = 0;

    if (argc == 0) {
        newNative = new Gtk::ColorSelection();
        myNewObject = new JSColorSelection(OWNERPTR(newNative), newNative);
    } else {
        JS_ReportError(cx,"Constructor for %s: bad number of arguments: expected none or two () %d",ClassName(), argc);
        return JS_FALSE;
    }

    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSColorSelection::Constructor: bad parameters");
    return JS_FALSE;
}
/*
JSConstIntPropertySpec *
JSColorSelection::ConstIntProperties() {

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
JSColorSelection::addClassProperties(JSContext * cx, JSObject * theClassProto) {
    JSBASE::addClassProperties(cx, theClassProto);
    JSA_AddFunctions(cx, theClassProto, Functions());
    JSA_AddProperties(cx, theClassProto, Properties());
    createClassModuleDocumentation("gtk", ClassName(), Properties(), Functions(),
            0, 0, 0, JSBASE::ClassName());
}

JSObject *
JSColorSelection::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject * myClassObject = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, 0,0);
    if (myClassObject) {
        addClassProperties(cx, myClassObject);
    }
    jsval myConstructorFuncObjVal;
    if (JS_GetProperty(cx, theGlobalObject, ClassName(), &myConstructorFuncObjVal)) {
//        JSObject * myConstructorFuncObj = JSVAL_TO_OBJECT(myConstructorFuncObjVal);
//        JSA_DefineConstInts(cx, myConstructorFuncObj, ConstIntProperties());
    } else {
        cerr << "JSColorSelection::initClass: constructor function object not found, could not initialize static members"<<endl;
    }
    return myClassObject;
}

jsval as_jsval(JSContext *cx, JSColorSelection::OWNERPTR theOwner, JSColorSelection::NATIVE * theNative) {
    JSObject * myReturnObject = JSColorSelection::Construct(cx, theOwner, theNative);
    return OBJECT_TO_JSVAL(myReturnObject);
}

}
