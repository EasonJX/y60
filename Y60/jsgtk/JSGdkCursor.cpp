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
//   $RCSfile: JSGdkCursor.cpp,v $
//   $Author: david $
//   $Revision: 1.2 $
//   $Date: 2005/04/13 15:24:13 $
//
//
//=============================================================================

#include "JSGdkCursor.h"
#include "jsgtk.h"
#include <acgtk/CustomCursors.h>
#include <y60/JScppUtils.h>

#include <iostream>
#include <gdk/gdkkeysyms.h>
#include <gtkmm/window.h>

using namespace std;
using namespace asl;

namespace jslib {

static JSBool
initCustomCursors(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    ensureParamCount(argc, 0);
    acgtk::CustomCursors::init();
    return JS_TRUE;
}

JSFunctionSpec *
JSGdkCursor::StaticFunctions() {
    static JSFunctionSpec myFunctions[] = {
        /* name         native          nargs    */
        {"initCustomCursors", initCustomCursors, 0},
        {0}
    };
    return myFunctions;
};

JSBool
JSGdkCursor::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }

    NATIVE * newNative = 0;

    JSGdkCursor * myNewObject = 0;

    if(argc == 0) {
        newNative = new Gdk::Cursor;
        myNewObject = new JSGdkCursor(OWNERPTR(0), newNative);
    } else if (argc == 1) {
        int myCursorId;
        if ( convertFrom(cx, argv[0], myCursorId)) {
            newNative = new Gdk::Cursor(static_cast<Gdk::CursorType>(myCursorId));
        } else {
            JS_ReportError(cx,"Constructor for %s: expected an int cursor constant, like 'GdkCursor.CROSSHAIR' ()",ClassName());
            return JS_FALSE;
        }
        myNewObject = new JSGdkCursor(OWNERPTR(newNative), newNative);
    } else {
        JS_ReportError(cx,"Constructor for %s: bad number of arguments: expected one () %d",ClassName(), argc);
        return JS_FALSE;
    }

    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSGdkCursor::Constructor: bad parameters");
    return JS_FALSE;
}

// checkout http://www-eleves-isia.cma.fr/documentation/GtkDoc/gdk/gdk-cursors.html
// to get an overview
enum PropertyNumbers {
    PROP_X_CURSOR,
    PROP_ARROW,
    PROP_BASED_ARROW_DOWN,
    PROP_BASED_ARROW_UP,
    PROP_BOAT,
    PROP_BOGOSITY,
    PROP_BOTTOM_LEFT_CORNER,
    PROP_BOTTOM_RIGHT_CORNER,
    PROP_BOTTOM_SIDE,
    PROP_BOTTOM_TEE,
    PROP_BOX_SPIRAL,
    PROP_CENTER_PTR,
    PROP_CIRCLE,
    PROP_CLOCK,
    PROP_COFFEE_MUG,
    PROP_CROSS,
    PROP_CROSS_REVERSE,
    PROP_CROSSHAIR,
    PROP_DIAMOND_CROSS,
    PROP_DOT,
    PROP_DOTBOX,
    PROP_DOUBLE_ARROW,
    PROP_DRAFT_LARGE,
    PROP_DRAFT_SMALL,
    PROP_DRAPED_BOX,
    PROP_EXCHANGE,
    PROP_FLEUR,
    PROP_GOBBLER,
    PROP_GUMBY,
    PROP_HAND1,
    PROP_HAND2,
    PROP_HEART,
    PROP_ICON,
    PROP_IRON_CROSS,
    PROP_LEFT_PTR,
    PROP_LEFT_SIDE,
    PROP_LEFT_TEE,
    PROP_LEFTBUTTON,
    PROP_LL_ANGLE,
    PROP_LR_ANGLE,
    PROP_MAN,
    PROP_MIDDLEBUTTON,
    PROP_MOUSE,
    PROP_PENCIL,
    PROP_PIRATE,
    PROP_PLUS,
    PROP_QUESTION_ARROW,
    PROP_RIGHT_PTR,
    PROP_RIGHT_SIDE,
    PROP_RIGHT_TEE,
    PROP_RIGHTBUTTON,
    PROP_RTL_LOGO,
    PROP_SAILBOAT,
    PROP_SB_DOWN_ARROW,
    PROP_SB_H_DOUBLE_ARROW,
    PROP_SB_LEFT_ARROW,
    PROP_SB_RIGHT_ARROW,
    PROP_SB_UP_ARROW,
    PROP_SB_V_DOUBLE_ARROW,
    PROP_SHUTTLE,
    PROP_SIZING,
    PROP_SPIDER,
    PROP_SPRAYCAN,
    PROP_STAR,
    PROP_TARGET,
    PROP_TCROSS,
    PROP_TOP_LEFT_ARROW,
    PROP_TOP_LEFT_CORNER,
    PROP_TOP_RIGHT_CORNER,
    PROP_TOP_SIDE,
    PROP_TOP_TEE,
    PROP_TREK,
    PROP_UL_ANGLE,
    PROP_UMBRELLA,
    PROP_UR_ANGLE,
    PROP_WATCH,
    PROP_XTERM
};

enum StaticPropertyNumbers {
    PROP_AC_ADD_POINT
};

JSPropertySpec *
JSGdkCursor::Properties() {
    static JSPropertySpec myProperties[] = {
        {0}
    };
    return myProperties;
}

#define DEFINE_GDK_PROP( NAME ) { #NAME, PROP_ ## NAME, Gdk::NAME}

// checkout http://www-eleves-isia.cma.fr/documentation/GtkDoc/gdk/gdk-cursors.html
// to get an overview
JSConstIntPropertySpec *
JSGdkCursor::ConstIntProperties() {
    static JSConstIntPropertySpec myProperties[] = {
        // types
        DEFINE_GDK_PROP(X_CURSOR),
        DEFINE_GDK_PROP(ARROW),
        DEFINE_GDK_PROP(BASED_ARROW_DOWN),
        DEFINE_GDK_PROP(BASED_ARROW_UP),
        DEFINE_GDK_PROP(BOAT),
        DEFINE_GDK_PROP(BOGOSITY),
        DEFINE_GDK_PROP(BOTTOM_LEFT_CORNER),
        DEFINE_GDK_PROP(BOTTOM_RIGHT_CORNER),
        DEFINE_GDK_PROP(BOTTOM_SIDE),
        DEFINE_GDK_PROP(BOTTOM_TEE),
        DEFINE_GDK_PROP(BOX_SPIRAL),
        DEFINE_GDK_PROP(CENTER_PTR),
        DEFINE_GDK_PROP(CIRCLE),
        DEFINE_GDK_PROP(CLOCK),
        DEFINE_GDK_PROP(COFFEE_MUG),
        DEFINE_GDK_PROP(CROSS),
        DEFINE_GDK_PROP(CROSS_REVERSE),
        DEFINE_GDK_PROP(CROSSHAIR),
        DEFINE_GDK_PROP(DIAMOND_CROSS),
        DEFINE_GDK_PROP(DOT),
        DEFINE_GDK_PROP(DOTBOX),
        DEFINE_GDK_PROP(DOUBLE_ARROW),
        DEFINE_GDK_PROP(DRAFT_LARGE),
        DEFINE_GDK_PROP(DRAFT_SMALL),
        DEFINE_GDK_PROP(DRAPED_BOX),
        DEFINE_GDK_PROP(EXCHANGE),
        DEFINE_GDK_PROP(FLEUR),
        DEFINE_GDK_PROP(GOBBLER),
        DEFINE_GDK_PROP(GUMBY),
        DEFINE_GDK_PROP(HAND1),
        DEFINE_GDK_PROP(HAND2),
        DEFINE_GDK_PROP(HEART),
        DEFINE_GDK_PROP(ICON),
        DEFINE_GDK_PROP(IRON_CROSS),
        DEFINE_GDK_PROP(LEFT_PTR),
        DEFINE_GDK_PROP(LEFT_SIDE),
        DEFINE_GDK_PROP(LEFT_TEE),
        DEFINE_GDK_PROP(LEFTBUTTON),
        DEFINE_GDK_PROP(LL_ANGLE),
        DEFINE_GDK_PROP(LR_ANGLE),
        DEFINE_GDK_PROP(MAN),
        DEFINE_GDK_PROP(MIDDLEBUTTON),
        DEFINE_GDK_PROP(MOUSE),
        DEFINE_GDK_PROP(PENCIL),
        DEFINE_GDK_PROP(PIRATE),
        DEFINE_GDK_PROP(PLUS),
        DEFINE_GDK_PROP(QUESTION_ARROW),
        DEFINE_GDK_PROP(RIGHT_PTR),
        DEFINE_GDK_PROP(RIGHT_SIDE),
        DEFINE_GDK_PROP(RIGHT_TEE),
        DEFINE_GDK_PROP(RIGHTBUTTON),
        DEFINE_GDK_PROP(RTL_LOGO),
        DEFINE_GDK_PROP(SAILBOAT),
        DEFINE_GDK_PROP(SB_DOWN_ARROW),
        DEFINE_GDK_PROP(SB_H_DOUBLE_ARROW),
        DEFINE_GDK_PROP(SB_LEFT_ARROW),
        DEFINE_GDK_PROP(SB_RIGHT_ARROW),
        DEFINE_GDK_PROP(SB_UP_ARROW),
        DEFINE_GDK_PROP(SB_V_DOUBLE_ARROW),
        DEFINE_GDK_PROP(SHUTTLE),
        DEFINE_GDK_PROP(SIZING),
        DEFINE_GDK_PROP(SPIDER),
        DEFINE_GDK_PROP(SPRAYCAN),
        DEFINE_GDK_PROP(STAR),
        DEFINE_GDK_PROP(TARGET),
        DEFINE_GDK_PROP(TCROSS),
        DEFINE_GDK_PROP(TOP_LEFT_ARROW),
        DEFINE_GDK_PROP(TOP_LEFT_CORNER),
        DEFINE_GDK_PROP(TOP_RIGHT_CORNER),
        DEFINE_GDK_PROP(TOP_SIDE),
        DEFINE_GDK_PROP(TOP_TEE),
        DEFINE_GDK_PROP(TREK),
        DEFINE_GDK_PROP(UL_ANGLE),
        DEFINE_GDK_PROP(UMBRELLA),
        DEFINE_GDK_PROP(UR_ANGLE),
        DEFINE_GDK_PROP(WATCH),
        DEFINE_GDK_PROP(XTERM)
    };
    return myProperties;
};

static JSBool
getStaticProperty(JSContext *cx, JSObject * obj, jsval id, jsval * vp) {
    int myID = JSVAL_TO_INT(id);
    switch (myID) {
        case PROP_AC_ADD_POINT:
            *vp = as_jsval(cx, JSGdkCursor::OWNERPTR(0), & acgtk::CustomCursors::AC_ADD_POINT);
            return JS_TRUE;
        default:
            JS_ReportError(cx,"JSGdkCursor::getStaticProperty: index %d out of range", myID);
            return JS_FALSE;
    }
    return JS_FALSE;
}

static JSBool
setStaticProperty(JSContext *cx, JSObject * obj, jsval id, jsval * vp) {
    int myID = JSVAL_TO_INT(id);
    switch (myID) {
        case 0:
        default:
            JS_ReportError(cx,"JSGdkCursor::setStaticProperty: index %d out of range", myID);
            return JS_FALSE;

    }
    return JS_FALSE;
}
#define DEFINE_STATIC_PROP(theName) \
    {#theName, PROP_ ## theName, JSPROP_READONLY | JSPROP_ENUMERATE | JSPROP_PERMANENT, \
            getStaticProperty, setStaticProperty}

JSPropertySpec *
JSGdkCursor::StaticProperties() {
    static JSPropertySpec myProperties[] = {
        DEFINE_STATIC_PROP(AC_ADD_POINT),
        {0}
    };
    return myProperties;
}


// getproperty handling
JSBool
JSGdkCursor::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    // common properties:
    switch (theID) {
        case 0:
            return JS_FALSE;
    }
    JS_ReportError(cx,"JSGdkCursor::getProperty: index %d out of range", theID);
    return JS_FALSE;
}

JSObject *
JSGdkCursor::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject * myClassObject = Base::initClass(cx, theGlobalObject, ClassName(), Constructor,
                                               Properties(), 0,
                                               0, StaticProperties(), StaticFunctions());
    jsval myConstructorFuncObjVal;
    if (JS_GetProperty(cx, theGlobalObject, ClassName(), &myConstructorFuncObjVal)) {
        JSObject * myConstructorFuncObj = JSVAL_TO_OBJECT(myConstructorFuncObjVal);
        JSA_DefineConstInts(cx, myConstructorFuncObj, ConstIntProperties());
    } else {
        cerr << "JSGdkCursor::initClass: constructor function object not found, could not initialize static members"<<endl;
    }
    createClassModuleDocumentation("gtk", ClassName(), Properties(), 0, ConstIntProperties(), 0, 0, 0);
    return myClassObject;
}

jsval as_jsval(JSContext *cx, JSGdkCursor::OWNERPTR theOwner, JSGdkCursor::NATIVE * theNative) {
    JSObject * myReturnObject = JSGdkCursor::Construct(cx, theOwner, theNative);
    return OBJECT_TO_JSVAL(myReturnObject);
}

bool convertFrom(JSContext *cx, jsval theValue, JSGdkCursor::NATIVE * & theCursor) {
    if (JSVAL_IS_OBJECT(theValue)) {
        JSObject * myArgument;
        if (JS_ValueToObject(cx, theValue, &myArgument)) {

            if (JSA_GetClass(cx,myArgument) == JSClassTraits<JSGdkCursor::NATIVE >::Class()) {
                typedef JSClassTraits<JSGdkCursor::NATIVE>::ScopedNativeRef NativeRef;
                NativeRef myObj(cx, myArgument);
                theCursor = & myObj.getNative();

                return true;
            }
        }
    }
    return false;
}


}
