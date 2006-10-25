//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include <Y60/JSVector.h>
#include "JSBox2f.h"
#include <Y60/JSWrapper.impl>
#include <asl/Box.h>

#include <iostream>

using namespace std;

namespace jslib {

template class JSWrapper<asl::Box2<BoxNumber> >;

typedef BoxNumber Number;
typedef asl::Box2<Number> NATIVE;

static JSBool
makeEmpty(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Makes this Box contain nothing.");
    DOC_END;
    return Method<NATIVE>::call(&NATIVE::makeEmpty,cx,obj,argc,argv,rval);
}
static JSBool
makeFull(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Makes this Box contain anything.");
    DOC_END;
    return Method<NATIVE>::call(&NATIVE::makeFull,cx,obj,argc,argv,rval);
}
static JSBool
makeCorrect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Flips Coordinates so that the Box has a proper upper near left and lower far right corner.");
    DOC_END;
    return Method<NATIVE>::call(&NATIVE::makeCorrect,cx,obj,argc,argv,rval);
}
static JSBool
extendBy(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Extend the Box to minimally include a given point/box.");
    DOC_PARAM("theExtensionPoint", "a point defining the resulting extension", DOC_TYPE_POINT2F);
    DOC_RESET;
    DOC_PARAM("theExtensionBox", "a box defining the resulting extension", DOC_TYPE_POINT2F);
    DOC_END;
    if (argc == 1) {
        if (JSBox2f::matchesClassOf(cx, argv[0])) {
            typedef void (NATIVE::*MyMethod)(const asl::Box2<Number> &);
            return Method<NATIVE>::call((MyMethod)&NATIVE::extendBy,cx,obj,argc,argv,rval);
        } else {
            typedef void (NATIVE::*MyMethod)(const asl::Point2<Number> &);
            return Method<NATIVE>::call((MyMethod)&NATIVE::extendBy,cx,obj,argc,argv,rval);
        }
    }
    JS_ReportError(cx,"JSBox2f::extendBy: bad number of arguments, 1 expected");
    return JS_FALSE;
}

static JSBool
intersects(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Intersect the Box with another one.");
    DOC_PARAM("theIntersectingBox", "", DOC_TYPE_BOX2F);
    DOC_END;
    if (argc == 1) {
        if (JSBox2f::matchesClassOf(cx, argv[0])) {
            typedef bool (NATIVE::*MyMethod)(const asl::Box2<Number> &) const;
            return Method<NATIVE>::call((MyMethod)&NATIVE::intersects,cx,obj,argc,argv,rval);
        } else {
            JS_ReportError(cx,"JSBox2f::intersects: bad arguments, Box2f expected");
            return JS_FALSE;
        }
    }
    JS_ReportError(cx,"JSBox2f::intersects: bad number of arguments, 1 expected");
    return JS_FALSE;
}
static JSBool
contains(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Check if the Box contains a point/box.");
    DOC_PARAM("theContainedPoint", "", DOC_TYPE_POINT2F);
    DOC_RESET;
    DOC_PARAM("theContainedBox", "", DOC_TYPE_BOX2F);
    DOC_RVAL("true/false", DOC_TYPE_BOOLEAN);
    DOC_END;
    if (argc == 1) {
        if (JSBox2f::matchesClassOf(cx, argv[0])) {
            typedef bool (NATIVE::*MyMethod)(const asl::Box2<Number> &) const;
            return Method<NATIVE>::call((MyMethod)&NATIVE::contains,cx,obj,argc,argv,rval);
        } else {
            typedef bool (NATIVE::*MyMethod)(const asl::Point2<Number> &) const;
            return Method<NATIVE>::call((MyMethod)&NATIVE::contains,cx,obj,argc,argv,rval);
        }
    }
    JS_ReportError(cx,"JSBox2f::contains: bad number of arguments, 1 expected");
    return JS_FALSE;
}

static JSBool
envelopes(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Check if the Box contains and doesn't touch a point/box.");
    DOC_PARAM("theEnvelopedPoint", "", DOC_TYPE_POINT2F);
    DOC_RESET;
    DOC_PARAM("theEnvelopedBox", "", DOC_TYPE_BOX2F);
    DOC_RVAL("true/false", DOC_TYPE_BOOLEAN);
    DOC_END;
    if (argc == 1) {
        if (JSBox2f::matchesClassOf(cx, argv[0])) {
            typedef bool (NATIVE::*MyMethod)(const asl::Box2<Number> &) const;
            return Method<NATIVE>::call((MyMethod)&NATIVE::envelopes,cx,obj,argc,argv,rval);
        } else {
            typedef bool (NATIVE::*MyMethod)(const asl::Point2<Number> &) const;
            return Method<NATIVE>::call((MyMethod)&NATIVE::envelopes,cx,obj,argc,argv,rval);
        }
    }
    JS_ReportError(cx,"JSBox2f::envelopes: bad number of arguments, 1 expected");
    return JS_FALSE;
}
static JSBool
touches(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Check if the Box touches a point/box.");
    DOC_PARAM("theTouchingPoint", "", DOC_TYPE_POINT2F);
    DOC_RESET;
    DOC_PARAM("theTouchingBox", "", DOC_TYPE_BOX2F);
    DOC_RVAL("true/false", DOC_TYPE_BOOLEAN);
    DOC_END;
    if (argc == 1) {
        if (JSBox2f::matchesClassOf(cx, argv[0])) {
            typedef bool (NATIVE::*MyMethod)(const asl::Box2<Number> &) const;
            return Method<NATIVE>::call((MyMethod)&NATIVE::touches,cx,obj,argc,argv,rval);
        } else {
            typedef bool (NATIVE::*MyMethod)(const asl::Point2<Number> &) const;
            return Method<NATIVE>::call((MyMethod)&NATIVE::touches,cx,obj,argc,argv,rval);
        }
    }
    JS_ReportError(cx,"JSBox2f::touches: bad number of arguments, 1 expected");
    return JS_FALSE;
}
static JSBool
setBounds(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("set the bounds of the Box.");
    DOC_PARAM("theUpperNearLeftCorner", "", DOC_TYPE_POINT2F);
    DOC_PARAM("theLowerFarRightCorner", "", DOC_TYPE_POINT2F);
    DOC_END;
    typedef void (NATIVE::*MyMethod)(const asl::Point2<Number> &, const asl::Point2<Number> &);
    return Method<NATIVE>::call((MyMethod)&NATIVE::setBounds,cx,obj,argc,argv,rval);
}

static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("returns string representation of the Box.");
    DOC_RVAL("", DOC_TYPE_STRING);
    DOC_END;
    std::string myStringRep = asl::as_string(JSBox2f::getJSWrapper(cx,obj).getNative());
*rval = as_jsval(cx, myStringRep);
    return JS_TRUE;
}

JSFunctionSpec *
JSBox2f::Functions() {
    AC_DEBUG << "Registering class '"<<ClassName()<<"'"<<endl;
    static JSFunctionSpec myFunctions[] = {
        /* name                native          nargs    */
        {"makeEmpty",          makeEmpty,               0},
        {"makeFull",           makeFull,                0},
        {"makeCorrect",        makeCorrect,             0},
        {"extendBy",           extendBy,                1}, // point, box
        {"contains",           contains,                0}, // point, box
        {"intersects",         intersects,                 1}, // box
        {"envelopes",          envelopes,               1},
        {"touches",            touches,                 1},
        {"setBounds",          setBounds,               2},
        {"toString",           toString,                0},
        {0}
    };
    return myFunctions;
}

JSPropertySpec *
JSBox2f::Properties() {
    static JSPropertySpec myProperties[] = {
        {"length",      PROP_length,      JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},   // readonly attribute unsigned long
        {"hasPosition", PROP_hasPosition, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},   // readonly attribute boolean
        {"hasSize",     PROP_hasSize,     JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},     // readonly attribute boolean
        {"hasArea",     PROP_hasArea,     JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},     // readonly attribute boolean
        {"hasVolume",   PROP_hasVolume,   JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},     // readonly attribute boolean
        {"isEmpty",     PROP_isEmpty,     JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},     // readonly attribute boolean
        {"min",         PROP_min,         JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED},     // attribute point
        {"max",         PROP_max,         JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED},     // attribute point
        {"center",      PROP_center,      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED},     // attribute point
        {"size",        PROP_size,        JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},     // readonly attribute vector
        {"volume",      PROP_volume,      JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},     // readonly attribute number
        {"area",        PROP_area,        JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},     // readonly attribute boolean
        {0}
    };
    return myProperties;
}

JSConstIntPropertySpec *
JSBox2f::ConstIntProperties() {
    static JSConstIntPropertySpec myProperties[] = {{0}};
    return myProperties;
}

JSPropertySpec *
JSBox2f::StaticProperties() {
    static JSPropertySpec myProperties[] = {{0}};
    return myProperties;
}

JSFunctionSpec *
JSBox2f::StaticFunctions() {
    static JSFunctionSpec myFunctions[] = {{0}};
    return myFunctions;
}


// getproperty handling
JSBool
JSBox2f::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
            case PROP_length:
                *vp = as_jsval(cx, 2);
                return JS_TRUE;
            case PROP_hasPosition:
                *vp = as_jsval(cx, getNative().hasPosition());
                return JS_TRUE;
            case PROP_hasSize:
                *vp = as_jsval(cx, getNative().hasSize());
                return JS_TRUE;
            case PROP_hasArea:
                *vp = as_jsval(cx, getNative().hasArea());
                return JS_TRUE;
            case PROP_hasVolume:
                *vp = as_jsval(cx, getNative().hasVolume());
                return JS_TRUE;
            case PROP_isEmpty:
                *vp = as_jsval(cx, getNative().isEmpty());
                return JS_TRUE;
            case PROP_min:
                *vp = as_jsval(cx, getNative().getMin());
                return JS_TRUE;
            case PROP_max:
                *vp = as_jsval(cx, getNative().getMax());
                return JS_TRUE;
            case PROP_center:
                *vp = as_jsval(cx, getNative().getCenter());
                return JS_TRUE;
            case PROP_size:
                *vp = as_jsval(cx, getNative().getSize());
                return JS_TRUE;
            case PROP_volume:
                *vp = as_jsval(cx, getNative().getVolume());
                return JS_TRUE;
            case PROP_area:
                *vp = as_jsval(cx, getNative().getArea());
                return JS_TRUE;
            default:
                JS_ReportError(cx,"JSBox2f::getProperty: index %d out of range", theID);
                return JS_FALSE;
    }
};
JSBool JSBox2f::getPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    *vp = as_jsval(cx, getNative()[theIndex]);
    return JS_TRUE;
};

// setproperty handling
JSBool
JSBox2f::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    jsval dummy;
    switch (theID) {
        case PROP_min:
            return Method<NATIVE>::call(&NATIVE::setMin, cx, obj, 1, vp, &dummy);
        case PROP_max:
            return Method<NATIVE>::call(&NATIVE::setMax, cx, obj, 1, vp, &dummy);
        case PROP_center:
            return Method<NATIVE>::call(&NATIVE::setCenter, cx, obj, 1, vp, &dummy);
        default:
            JS_ReportError(cx,"JSBox2f::setPropertySwitch: index %d out of range", theID);
            return JS_FALSE;
    }
};

JSBool
JSBox2f::setPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    if (JSVAL_IS_OBJECT(*vp)) {
        JSObject * myValObj = JSVAL_TO_OBJECT(*vp);
        if (JSVector<asl::Vector2<Number> >::Class() != JSA_GetClass(cx,myValObj)) {
            myValObj = JSVector<asl::Point2<Number> >::Construct(cx, *vp);
        }
        if (myValObj) {
            const asl::Point2<Number> & myNativeArg = JSClassTraits<asl::Point2<Number> >::getNativeRef(cx, JSVAL_TO_OBJECT(*vp));
            openNative()[theIndex] = myNativeArg;
            closeNative();
            return JS_TRUE;
        } else {
            JS_ReportError(cx,"JSBox2f::setPropertyIndex: bad argument type, Point2 expected");
            return JS_FALSE;
        }
    }
    return JS_TRUE;
};

JSBool
JSBox2f::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Constructs a new box object");
    DOC_RESET;
    DOC_PARAM("theCorner", "One corner of the box", DOC_TYPE_VECTOR2F);
    DOC_PARAM("theCorner", "Another corner of the box", DOC_TYPE_VECTOR2F);
    DOC_RESET;
    DOC_PARAM("theOtherBox", "Construct from another box (copy constructor)", DOC_TYPE_BOX2F);
    DOC_END;
    IF_NOISY2(AC_TRACE << "Constructor argc =" << argc << endl);
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }
    JSBox2f * myNewObject = 0;
    JSBox2f::NativeValuePtr myNewValue = JSBox2f::NativeValuePtr(new dom::SimpleValue<asl::Box2<Number> >(0));
    asl::Box2<Number> & myNewBox2f = myNewValue->openWriteableValue(); // we close it only on success, otherwise we trash it anyway
    if (argc == 0) {
        // construct empty
        myNewObject=new JSBox2f(myNewValue);
    } else {
        if (argc == 2) {
            // construct from two points
            for (int i = 0; i < 2 ;++i) {
                JSObject * myObject = JSVector<asl::Vector2<Number> >::Construct(cx, argv[i]);
                if (!myObject) {
                    JS_ReportError(cx,"JSBox2f::Constructor: argument must be 2 vectors or arrays of size 2",ClassName());
                    return JS_FALSE;
                }
                (myNewBox2f)[i] = JSClassTraits<asl::Vector2<Number> >::getNativeRef(cx,myObject);
            }
            myNewObject=new JSBox2f(myNewValue);
        } else if (argc == 1) {
            // construct from one Box2f
            JSObject * myArgument;
            if (!JS_ValueToObject(cx, argv[0], &myArgument)) {
                JS_ReportError(cx,"JSBox2f::Constructor: bad argument type, Box2f expected");
                return JS_FALSE;
            }
            if (JSA_GetClass(cx,myArgument) == Class()) {
                myNewObject=new JSBox2f(myNewValue);
            }
        } else {
            JS_ReportError(cx,"Constructor for %s: bad number of arguments: expected 0,1 or 3, got %d",ClassName(), argc);
            return JS_FALSE;
        }
    }
    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        myNewValue->closeWriteableValue();
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSBox2f::Constructor: bad parameters");
    return JS_FALSE;
}

JSObject *
JSBox2f::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject *myClass = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, Properties(), Functions());
    DOC_MODULE_CREATE("Math", JSBox2f);
    return myClass;
}

bool convertFrom(JSContext *cx, jsval theValue, asl::Box2<Number>  & theBox2f) {
    if (JSVAL_IS_OBJECT(theValue)) {
        JSObject * myArgument;
        if (JS_ValueToObject(cx, theValue, &myArgument)) {
            if (JSA_GetClass(cx,myArgument) == JSClassTraits<asl::Box2<Number> >::Class()) {
                theBox2f = JSClassTraits<asl::Box2<Number> >::getNativeRef(cx,myArgument);
                return true;
            }
        }
    }
    return false;
}

jsval as_jsval(JSContext *cx, const asl::Box2<Number>  & theValue) {
    JSObject * myReturnObject = JSBox2f::Construct(cx, theValue);
    return OBJECT_TO_JSVAL(myReturnObject);
}

jsval as_jsval(JSContext *cx, JSBox2f::NativeValuePtr theValue) {
    JSObject * myObject = JSBox2f::Construct(cx, theValue);
    return OBJECT_TO_JSVAL(myObject);
}

}
