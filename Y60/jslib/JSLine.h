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
//   $RCSfile: JSLine.h,v $
//   $Author: christian $
//   $Revision: 1.6 $
//   $Date: 2005/04/28 17:12:58 $
//
//
//=============================================================================

#ifndef _Y60_ACXPSHELL_JSLine_H_INCLUDED_
#define _Y60_ACXPSHELL_JSLine_H_INCLUDED_

#include "JSWrapper.h"
#include "JSVector.h"

#include <asl/Vector234.h>
#include <asl/numeric_functions.h>
#include <asl/linearAlgebra.h>
#include <asl/Line.h>
#include <dom/Value.h>


namespace jslib {

typedef float LineNumber;
typedef float RayNumber;
typedef float LineSegmentNumber;

template <class NATIVE_LINE>
class JSGenericLine : public JSWrapper<NATIVE_LINE>
{
    typedef JSWrapper<NATIVE_LINE> Base;
    JSGenericLine() {}
public:
    static const char * ClassName();
    static JSFunctionSpec * Functions();

    enum PropertyNumbers {
        PROP_origin = -100,
        PROP_direction,
        PROP_end
    };
    static JSPropertySpec * Properties();
    static JSConstIntPropertySpec * ConstIntProperties();
    static JSPropertySpec * StaticProperties();
    static JSFunctionSpec * StaticFunctions();


    // getproperty handling
    virtual JSBool getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);
    //virtual JSBool getPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp);

    // setproperty handling
    virtual JSBool setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);
    //virtual JSBool setPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp);

    static JSBool
    Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

    static
    JSObject * Construct(JSContext *cx, typename JSGenericLine<NATIVE_LINE>::NativeValuePtr theValue) {
        return Base::Construct(cx, theValue, 0);
    }
    static
    JSObject * Construct(JSContext *cx, const NATIVE_LINE & theLine) {
        typename JSGenericLine<NATIVE_LINE>::NativeValuePtr myValue(new dom::SimpleValue<NATIVE_LINE >(theLine, 0));
        return Base::Construct(cx, myValue, 0);
    }
    JSGenericLine(typename JSGenericLine<NATIVE_LINE>::NativeValuePtr theValue)
        : Base(theValue, 0)
    {}


    static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);
};


typedef JSGenericLine<asl::Line<LineNumber> > JSLine;
typedef JSGenericLine<asl::Ray<LineNumber> > JSRay;
typedef JSGenericLine<asl::LineSegment<LineNumber> > JSLineSegment;

template <class NATIVE_LINE>
struct JSLineClassTraits : public JSClassTraitsWrapper<NATIVE_LINE, JSGenericLine<NATIVE_LINE> > {};

template <>
struct JSClassTraits<asl::Line<LineNumber> > : public JSLineClassTraits<asl::Line<LineNumber> > {
};
template <>
struct JSClassTraits<asl::Ray<LineNumber> > : public JSLineClassTraits<asl::Ray<LineNumber> > {
};
template <>
struct JSClassTraits<asl::LineSegment<LineNumber> > : public JSLineClassTraits<asl::LineSegment<LineNumber> > {
};


bool convertFrom(JSContext *cx, jsval theValue, asl::Line<LineNumber> & theLine);
jsval as_jsval(JSContext *cx, const asl::Line<LineNumber>  & theValue);
//jsval as_jsval(JSContext *cx, dom::ValuePtr theValue, asl::Line<LineNumber> * theLine);
jsval as_jsval(JSContext *cx, JSLine::NativeValuePtr theLine);

bool convertFrom(JSContext *cx, jsval theValue, asl::Ray<LineNumber> & theLine);
jsval as_jsval(JSContext *cx, const asl::Ray<LineNumber>  & theValue);
//jsval as_jsval(JSContext *cx, dom::ValuePtr theValue, asl::Ray<LineNumber> * theLine);
jsval as_jsval(JSContext *cx, JSRay::NativeValuePtr theLine);

bool convertFrom(JSContext *cx, jsval theValue, asl::LineSegment<LineNumber> & theLine);
jsval as_jsval(JSContext *cx, const asl::LineSegment<LineNumber> & theValue);
//jsval as_jsval(JSContext *cx, dom::ValuePtr theValue, asl::LineSegment<LineNumber> * theLine);
jsval as_jsval(JSContext *cx, JSLineSegment::NativeValuePtr theLine);


template <class NATIVE_LINE>
static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("returns a string representation. e.g. '[[0,0,0],[100,100,100]]'");
    DOC_END;
    std::string myStringRep = asl::as_string(JSGenericLine<NATIVE_LINE>::getJSWrapper(cx,obj).getNative());
    JSString * myString = JS_NewStringCopyN(cx,myStringRep.c_str(),myStringRep.size());
    *rval = STRING_TO_JSVAL(myString);
    return JS_TRUE;
}


template <class NATIVE_LINE>
JSFunctionSpec *
JSGenericLine<NATIVE_LINE>::Functions() {
    AC_DEBUG << "Registering class '"<<ClassName()<<"'"<<std::endl;
    static JSFunctionSpec myFunctions[] = {
        /* name                native          nargs    */
        {"toString",           &toString<NATIVE_LINE>,                0},
        {0}
    };
    return myFunctions;
}

template <class NATIVE_LINE>
JSPropertySpec *
JSGenericLine<NATIVE_LINE>::Properties() {
    static JSPropertySpec myProperties[] = {
        {"origin", PROP_origin, JSPROP_ENUMERATE|JSPROP_PERMANENT},     // readwrite attribute Point3f
        {"direction", PROP_direction, JSPROP_ENUMERATE|JSPROP_PERMANENT},     // readwrite attribute Vector3f
        {0}
    };
    return myProperties;
}

template <class NATIVE_LINE>
JSConstIntPropertySpec *
JSGenericLine<NATIVE_LINE>::ConstIntProperties() {
    static JSConstIntPropertySpec myProperties[] = {{0}};
    return myProperties;
}

template <class NATIVE_LINE>
JSPropertySpec *
JSGenericLine<NATIVE_LINE>::StaticProperties() {
    static JSPropertySpec myProperties[] = {{0}};
    return myProperties;
}

template <class NATIVE_LINE>
JSFunctionSpec *
JSGenericLine<NATIVE_LINE>::StaticFunctions() {
    static JSFunctionSpec myFunctions[] = {{0}};
    return myFunctions;
}
 

template<>
JSPropertySpec *
JSGenericLine<asl::LineSegment<LineNumber> >::Properties();

// getproperty handling
template <class NATIVE_LINE>
JSBool
JSGenericLine<NATIVE_LINE>::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
            case PROP_origin:
                *vp = as_jsval(cx, this->getNative().origin);
                return JS_TRUE;
            case PROP_direction:
                *vp = as_jsval(cx, this->getNative().direction);
                return JS_TRUE;
            default:
                JS_ReportError(cx,"JSLine::getProperty: index %d out of range", theID);
                return JS_FALSE;
    }
};

template <>
JSBool
JSGenericLine<asl::LineSegment<LineNumber> >::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);

// setproperty handling
template <class NATIVE_LINE>
JSBool
JSGenericLine<NATIVE_LINE>::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    jsval dummy;
    switch (theID) {
        case PROP_origin:
            return Method<NATIVE_LINE>::call(&NATIVE_LINE::setOrigin, cx, obj, 1, vp, &dummy);
        case PROP_direction:
            return Method<NATIVE_LINE>::call(&NATIVE_LINE::setDirection, cx, obj, 1, vp, &dummy);
        default:
            JS_ReportError(cx,"JSLine::setPropertySwitch: index %d out of range", theID);
            return JS_FALSE;
    }
};

template <>
JSBool
JSGenericLine<asl::LineSegment<LineNumber> >::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);

template <class NATIVE_LINE>
JSBool
JSGenericLine<NATIVE_LINE>::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Construct a Line / LineSegment / Ray");
    DOC_PARAM("thePointA", "A point through the Line. The start point of the Ray / LineSegment.", DOC_TYPE_POINT3F);
    DOC_PARAM("thePointB", "Another point through the Line / Ray. The end point of the LineSegment.", DOC_TYPE_POINT3F);
    DOC_RESET;
    DOC_PARAM("thePoint", "A point through the Line / The origin of the Ray.", DOC_TYPE_POINT3F);
    DOC_PARAM("theDirection", "The direction vector of the Line / Ray.", DOC_TYPE_VECTOR3F);
    DOC_END;
    IF_NOISY2(AC_TRACE << "Constructor argc =" << argc << std::endl);
    if (JSA_GetClass(cx,obj) != Base::Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }
    JSGenericLine<NATIVE_LINE> * myNewObject = 0;
    typename JSGenericLine<NATIVE_LINE>::NativeValuePtr myNewValue =
        typename JSGenericLine<NATIVE_LINE>::NativeValuePtr(new dom::SimpleValue<NATIVE_LINE>(0));
    NATIVE_LINE & myNewLine = myNewValue->openWriteableValue(); // we close it only on success, otherwise we trash it anyway
    if (argc == 0) {
        // construct empty
        myNewObject=new JSGenericLine<NATIVE_LINE>(myNewValue);
    } else {
        if (argc == 2) {
            // construct from origin point and direction vector
            JSObject * myObject = JSVector<asl::Point3<LineNumber> >::Construct(cx, argv[0]);
            if (!myObject) {
                JS_ReportError(cx,"JSGenericLine<%s>::Constructor: first argument must be an origin point of size 3",ClassName());
                return JS_FALSE;
            }
            if (JSVAL_IS_VOID(argv[1])) {
                JS_ReportError(cx,"JSGenericLine<%s>::Constructor: bad argument #2 (undefined)",ClassName());
                return JS_FALSE;
            }
            JSObject * myArgument;
            if (!JS_ValueToObject(cx, argv[1], &myArgument)) {
                JS_ReportError(cx,"JSGenericLine<%s>::Constructor: bad argument type, object (point3 or vector3) expected",ClassName());
                return JS_FALSE;
            }
            if (JSA_GetClass(cx,myArgument) == JSVector<asl::Point3<LineNumber> >::Class()) {
                // use point/point constructor
                myNewLine = NATIVE_LINE(JSVector<asl::Point3<LineNumber> >::getNativeRef(cx,myObject),
                                        JSVector<asl::Point3<LineNumber> >::getNativeRef(cx, myArgument));
            } else {
                // origin point/direction vector constructor
                JSObject * myObject2 = JSVector<asl::Vector3<LineNumber> >::Construct(cx, argv[1]);
                if (!myObject2) {
                    JS_ReportError(cx,"JSGenericLine<%s>::Constructor: second argument must be a direction vector of size 3",ClassName());
                    return JS_FALSE;
                }
                myNewLine = NATIVE_LINE(JSVector<asl::Point3<LineNumber> >::getNativeRef(cx,myObject),
                                        JSVector<asl::Vector3<LineNumber> >::getNativeRef(cx,myObject2));
            }
            myNewObject=new JSGenericLine<NATIVE_LINE>(myNewValue);
        } else if (argc == 1) {
            // construct from one Line
            JSObject * myArgument;
            if (!JS_ValueToObject(cx, argv[0], &myArgument)) {
                JS_ReportError(cx,"JSGenericLine<%s>::Constructor: bad argument type, Line expected",ClassName());
                return JS_FALSE;
            }
            if (JSA_GetClass(cx,myArgument) == Base::Class()) {
                myNewLine = Base::getJSWrapper(cx, myArgument).getNative();
                myNewObject=new JSGenericLine<NATIVE_LINE>(myNewValue);
            }
        } else {
            JS_ReportError(cx,"JSGenericLine<%s>::Constructor: bad number of arguments: expected 0,1 or 2, got %d",ClassName(), argc);
            return JS_FALSE;
        }
    }
    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        myNewValue->closeWriteableValue();
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSGenericLine<%s>::Constructor: bad parameters",ClassName());
    return JS_FALSE;
}

template <class NATIVE_LINE>
JSObject *
JSGenericLine<NATIVE_LINE>::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject *myClass = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, Properties(), Functions());
    DOC_MODULE_CREATE("Math", JSGenericLine<NATIVE_LINE>);
    return myClass;

}

}

#endif

