/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2008, ART+COM AG Berlin, Germany <www.artcom.de>
//
// These coded instructions, statements, and computer programs contain
// proprietary information of ART+COM AG Berlin, and are copy protected
// by law. They may be used, modified and redistributed under the terms
// of GNU General Public License referenced below. 
//    
// Alternative licensing without the obligations of the GPL is
// available upon request.
//
// GPL v3 Licensing:
//
// This file is part of the ART+COM Y60 Platform.
//
// ART+COM Y60 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// ART+COM Y60 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with ART+COM Y60.  If not, see <http://www.gnu.org/licenses/>.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Description: TODO  
//
// Last Review: NEVER, NOONE
//
//  review status report: (perfect, ok, fair, poor, disaster, notapplicable, unknown)
//    usefullness            : unknown
//    formatting             : unknown
//    documentation          : unknown
//    test coverage          : unknown
//    names                  : unknown
//    style guide conformance: unknown
//    technical soundness    : unknown
//    dead code              : unknown
//    readability            : unknown
//    understandabilty       : unknown
//    interfaces             : unknown
//    confidence             : unknown
//    integration            : unknown
//    dependencies           : unknown
//    cheesyness             : unknown
//
//    overall review status  : unknown
//
//    recommendations: 
//       - unknown
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/

// own header
#include "JSResizeableVector.h"

#include "JSNode.h"
#include "JSWrapper.impl"

#include <asl/base/string_functions.h>
#include <iostream>

using namespace std;
using namespace asl;

namespace {
    
    template<typename Value>
    class NativeRef {
    public:
        NativeRef(JSContext *cx, JSObject *obj)
        : cx_(cx), obj_(obj), value_(jslib::JSClassTraits<Value>::openNativeRef(cx, obj))
        { }
        
        Value& getValue() {
            return value_;
        }
        
        ~NativeRef() {
            jslib::JSClassTraits<Value>::closeNativeRef(cx_, obj_);
        }
    private:
        JSContext *cx_;
        JSObject  *obj_;
        Value &    value_;
    };
    
}

namespace jslib {

typedef dom::ResizeableVector NATIVE_VECTOR;
//typedef JSWrapper<NATIVE_VECTOR,dom::ValuePtr> Base;

template class JSWrapper<dom::ResizeableVector, dom::ValuePtr, VectorValueAccessProtocol>;

static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Returns a string representation of the vector.");
    DOC_END;
    std::string myStringRep = asl::as_string(JSResizeableVector::getJSWrapper(cx,obj).getNative());
    *rval = as_jsval(cx, myStringRep);
    return JS_TRUE;
}

static JSBool
item(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Returns an element from the vector. Throws an exception, if index is out of bounds.");
    DOC_PARAM("theIndex", "Index of the element to retrieve.", DOC_TYPE_INTEGER);
    DOC_RVAL("theElement", DOC_TYPE_OBJECT);
    DOC_END;
    typedef dom::ValuePtr (NATIVE_VECTOR::*MyMethod)(int);
    return Method<NATIVE_VECTOR>::call((MyMethod)&NATIVE_VECTOR::getElement,cx,obj,argc,argv,rval);
}
static JSBool
resize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Resizes the vector to the requested size, adding default elements or removing superflous elements.");
    DOC_PARAM("theSize", "The new size of the vector", DOC_TYPE_INTEGER);
    DOC_END;
    typedef void (NATIVE_VECTOR::*MyMethod)(int);
    return Method<NATIVE_VECTOR>::call((MyMethod)&NATIVE_VECTOR::resize,cx,obj,argc,argv,rval);
}


static JSBool
append(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("appends an element to the vector");
    DOC_PARAM("theElement", "The element to append", DOC_TYPE_OBJECT);
    DOC_END;
    try {
        if (argc == 0) {
            JS_ReportError(cx, "append(theElement): needs an argument");
            return JS_FALSE;
        }
        
        NativeRef<dom::ResizeableVector> myNativeRef(cx,obj);
        dom::ResizeableVector & myNative = myNativeRef.getValue();
        dom::ValuePtr myArg;
        if (convertFrom(cx, argv[0], myNative.elementName(), myArg)) {
            myNative.append(*myArg);
        } else {
            JS_ReportError(cx, (string("JSResizeableVector::append:") + "could not convert argument value "
                                +as_string(cx, argv[0])+" to type " + myNative.elementName()).c_str());
            return JS_FALSE;
        }

    } HANDLE_CPP_EXCEPTION;
    return JS_TRUE;    
}

static JSBool
erase(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Erases one element from the vector");
    DOC_PARAM("theIndex", "The index of the element to erase", DOC_TYPE_INTEGER);
    DOC_RVAL("true, if successfull", DOC_TYPE_BOOLEAN);
    DOC_END;
    typedef bool (NATIVE_VECTOR::*MyMethod)(int);
    return Method<NATIVE_VECTOR>::call((MyMethod)&NATIVE_VECTOR::erase,cx,obj,argc,argv,rval);
}


static JSBool
insertBefore(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("insertBefore insert an element in the vector before thePosition");
    DOC_PARAM("thePosition", "The position to insert before", DOC_TYPE_INTEGER);
    DOC_PARAM("theElement", "The element to insert", DOC_TYPE_OBJECT);
    DOC_END;
    try {
        if (argc != 2) {
            JS_ReportError(cx, "insertBefore(thePos, theElement): needs two arguments");
            return JS_FALSE;
        }
        
        NativeRef<dom::ResizeableVector> myNativeRef(cx,obj);
        dom::ResizeableVector & myNative = myNativeRef.getValue();
        unsigned int myArg0;
        dom::ValuePtr myArg1;
        if (convertFrom(cx, argv[0], myArg0)) {
            if (convertFrom(cx, argv[1], myNative.elementName(), myArg1)) {
                myNative.insertBefore(myArg0, *myArg1);
            } else {
                JS_ReportError(cx, (string("JSResizeableVector::insertBefore:") + "could not convert second argumentvalue "
                                    +as_string(cx, argv[1])+" to type " + myNative.elementName()).c_str());
                return JS_FALSE;
            }
        } else {
            JS_ReportError(cx, (string("JSResizeableVector::insertBefore:") + "could not convert first argument value "
                                +as_string(cx, argv[0])+" to type unsigned integer").c_str());
            return JS_FALSE;
        }
        
    } HANDLE_CPP_EXCEPTION;
    return JS_TRUE;    
}


JSFunctionSpec *
JSResizeableVector::Functions() {
    AC_DEBUG << "Registering class '"<<ClassName()<<"'"<<endl;
    static JSFunctionSpec myFunctions[] = {
        /* name         native          nargs    */
        {"toString",         toString,        0},
        {"item",             item,            1},
        {"resize",           resize,          1},
        {"append",           append,          1},
        {"erase",            erase,           1},
        {"insertBefore",     insertBefore,    2},
        {0}
    };
    return myFunctions;
}
enum PropertyNumbers {PROP_length = -100};

JSPropertySpec *
JSResizeableVector::Properties() {
    static JSPropertySpec myProperties[] = {
        {"length", PROP_length, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},   // readonly attribute unsigned long
        {0}
    };
    return myProperties;
}

JSConstIntPropertySpec *
JSResizeableVector::ConstIntProperties() {
    static JSConstIntPropertySpec myProperties[] = {{0}};
    return myProperties;
}

JSPropertySpec *
JSResizeableVector::StaticProperties() {
    static JSPropertySpec myProperties[] = {{0}};
    return myProperties;
}

JSFunctionSpec *
JSResizeableVector::StaticFunctions() {
    static JSFunctionSpec myFunctions[] = {{0}};
    return myFunctions;
}

// getproperty handling
JSBool
JSResizeableVector::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
        case PROP_length:
            *vp = as_jsval(cx, getNative().length());
            return JS_TRUE;
        default:
            JS_ReportError(cx,"JSResizeableVector::getProperty: index %d out of range", theID);
            return JS_FALSE;
    }
}

JSBool JSResizeableVector::getPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    *vp = as_jsval(cx, getNative().getElement(theIndex));
    return JS_TRUE;
}

// setproperty handling
JSBool JSResizeableVector::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JS_ReportError(cx,"JSResizeableVector::setPropertySwitch: index %d out of range", theID);
    return JS_FALSE;
}

JSBool JSResizeableVector::setPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    dom::ValuePtr myArg;
    if (convertFrom(cx, *vp, getNative().elementName(), myArg)) {
        //AC_TRACE << "JSResizeableVector::setPropertyIndex theIndex =" << theIndex << " myArg: " << myArg->getString() << endl;
        bool mySuccess = openNative().setElement(theIndex,*myArg);
        closeNative();
        if (!mySuccess) {
            JS_ReportError(cx, (string("JSResizeableVector::setPropertyIndex:") +
                        " setElement failed: theIndex =" + as_string(theIndex) + " myArg: " +
                          myArg->getString()).c_str());
            return JS_FALSE;
        }
        return JS_TRUE;
    }
    JS_ReportError(cx, (string("JSResizeableVector::setPropertyIndex:")
                        + "convertFrom failed: theIndex = " + as_string(theIndex)).c_str());
    return JS_FALSE;
}

JSBool
JSResizeableVector::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Creats a new resizeable vector");
    DOC_END;
    IF_NOISY2(AC_TRACE << "Constructor argc =" << argc << endl);
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }
    JSResizeableVector * myNewObject = 0;
    if (argc == 0) {
        myNewObject=new JSResizeableVector();
    } else {
        JS_ReportError(cx,"Constructor for %s: superflous argument ignored",ClassName());
        myNewObject=new JSResizeableVector();
    }
    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSResizeableVector::Constructor: bad parameters");
    return JS_FALSE;
}


jsval as_jsval(JSContext *cx, dom::ValuePtr theValuePtr, dom::ResizeableVector *) {
    JSObject * myObject = JSResizeableVector::Construct(cx, theValuePtr);
    return OBJECT_TO_JSVAL(myObject);
}
}
