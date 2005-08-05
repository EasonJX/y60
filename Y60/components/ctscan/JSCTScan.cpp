//=============================================================================
// Copyright (C) 2003-2004, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: JSCTScan.cpp,v $
//   $Author: christian $
//   $Revision: 1.12 $
//   $Date: 2005/04/28 17:12:56 $
//
//
//=============================================================================

#include "JSCTScan.h"

#include <y60/JSApp.h>
#include <y60/JSVector.h>
#include <y60/JSBox.h>
#include <y60/JSNode.h>
#include <y60/Image.h>
#include <y60/JSScene.h>
#include <y60/JSSphere.h>
#include <y60/JSVector.h>
#include <dom/Nodes.h>

#include <asl/PackageManager.h>
#include <jsgtk/JSSignal2.h>

#include <iostream>

using namespace std;
using namespace asl;
using namespace y60;

#define DB(x) // x

namespace jslib {

static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    const CTScan & myNative = JSCTScan::getJSWrapper(cx,obj).getNative();
    std::string myStringRep = string("CTScan ");
    JSString * myString = JS_NewStringCopyN(cx,myStringRep.c_str(),myStringRep.size());
    *rval = STRING_TO_JSVAL(myString);
    return JS_TRUE;
}

static JSBool
loadSlices(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    try {
        JSClassTraits<CTScan>::ScopedNativeRef myObj(cx, obj);
        if (argc != 2) {
            JS_ReportError(cx, "JSCTScan::loadSlices(): Wrong number of arguments, "
                               "expected 2 (theSubDir, thePackage), got %d.", argc);
            return JS_FALSE;
        }
        string mySubDir;
        if (!JSVAL_IS_NULL(argv[0])) {
            convertFrom(cx, argv[0], mySubDir);
        }
        string myPackage;
        convertFrom(cx, argv[1], myPackage);

        PackageManager myPackageManager;
        myPackageManager.add(*JSApp::getPackageManager());
        myPackageManager.add(myPackage);

        CTScan & myCTScan = myObj.getNative();
        int slicesLoaded = myCTScan.loadSlices(myPackageManager, mySubDir, myPackage);
        *rval = as_jsval(cx, slicesLoaded);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
clear(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    return Method<JSCTScan::NATIVE>::call(&JSCTScan::NATIVE::clear,cx,obj,argc,argv,rval);
}

static JSBool
verifyCompleteness(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    return Method<JSCTScan::NATIVE>::call(&JSCTScan::NATIVE::verifyCompleteness,cx,obj,argc,argv,rval);
}

static JSBool
getVoxelDimensions(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    return Method<JSCTScan::NATIVE>::call(&JSCTScan::NATIVE::getVoxelDimensions,cx,obj,argc,argv,rval);
}

static JSBool
setVoxelSize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    return Method<JSCTScan::NATIVE>::call(&JSCTScan::NATIVE::setVoxelSize,cx,obj,argc,argv,rval);
}

static JSBool
getVoxelSize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    return Method<JSCTScan::NATIVE>::call(&JSCTScan::NATIVE::getVoxelSize,cx,obj,argc,argv,rval);
}

static JSBool
getValueRange(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    return Method<JSCTScan::NATIVE>::call(&JSCTScan::NATIVE::getValueRange,cx,obj,argc,argv,rval);
}

static JSBool
computeHistogram(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    try {
        JSClassTraits<CTScan>::ScopedNativeRef myObj(cx, obj);

        ensureParamCount(argc, 1);

        // have to convert a Box3f into a Box3i because Box3i isn't available in JS
        asl::Box3f myFloatBox;
        convertFrom(cx, argv[0], myFloatBox);
        asl::Box3i myVoxelBox(static_cast<int>( myFloatBox[Box3f::MIN][0]),
                              static_cast<int>( myFloatBox[Box3f::MIN][1]),
                              static_cast<int>( myFloatBox[Box3f::MIN][2]),
                              static_cast<int>( myFloatBox[Box3f::MAX][0]),
                              static_cast<int>( myFloatBox[Box3f::MAX][1]),
                              static_cast<int>( myFloatBox[Box3f::MAX][2]));

        std::vector<unsigned> myHistogram;
        
        CTScan & myCTScan = myObj.getNative();
        myCTScan.computeHistogram(myVoxelBox, myHistogram);
         *rval = as_jsval(cx, myHistogram);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}


static JSBool
reconstructToImage(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    try {
        JSClassTraits<CTScan>::ScopedNativeRef myObj(cx, obj);
        if (argc != 3) {
            JS_ReportError(cx, "JSCTScan::reconstructToImage(): Wrong number of arguments, "
                               "expected 3 (Orientation, SliceIndex, Image Node), got %d.", argc);
            return JS_FALSE;
        }
        if (JSVAL_IS_VOID(argv[0])) {
            JS_ReportError(cx, "JSCTScan::reconstructToImage(): Argument #0 is undefined");
            return JS_FALSE;
        }
        if (JSVAL_IS_VOID(argv[1])) {
            JS_ReportError(cx, "JSCTScan::reconstructToImage(): Argument #1 is undefined");
            return JS_FALSE;
        }
        if (JSVAL_IS_VOID(argv[2])) {
            JS_ReportError(cx, "JSCTScan::reconstructToImage(): Argument #2 is undefined");
            return JS_FALSE;
        }
        int myOrientation = 0;
        convertFrom(cx, argv[0], myOrientation);
        int mySlice = 0;
        convertFrom(cx, argv[1], mySlice);
        dom::NodePtr myImageNode;
        convertFrom(cx, argv[2], myImageNode);

        CTScan & myCTScan = myObj.getNative();
        myCTScan.reconstructToImage(CTScan::Orientation(myOrientation), mySlice, myImageNode);

        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
countTriangles(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    try {
        JSClassTraits<CTScan>::ScopedNativeRef myObj(cx, obj);

        ensureParamCount(argc, 3);

        // have to convert a Box3f into a Box3i because Box3i isn't available in JS
        asl::Box3f myFloatBox;
        convertFrom(cx, argv[0], myFloatBox);
        asl::Box3i myVoxelBox(static_cast<int>( myFloatBox[Box3f::MIN][0]),
                              static_cast<int>( myFloatBox[Box3f::MIN][1]),
                              static_cast<int>( myFloatBox[Box3f::MIN][2]),
                              static_cast<int>( myFloatBox[Box3f::MAX][0]),
                              static_cast<int>( myFloatBox[Box3f::MAX][1]),
                              static_cast<int>( myFloatBox[Box3f::MAX][2]));
        double myThreshold;
        convertFrom(cx, argv[1], myThreshold);
        int myDownSampleRate;
        convertFrom(cx, argv[2], myDownSampleRate);

        CTScan & myCTScan = myObj.getNative();
        int myTriangleCount = myCTScan.countTriangles(myVoxelBox, myThreshold, myDownSampleRate);
        *rval = as_jsval(cx, myTriangleCount);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}
static JSBool
polygonize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    try {
        JSClassTraits<CTScan>::ScopedNativeRef myObj(cx, obj);

        ensureParamCount(argc, 4);

        // have to convert a Box3f into a Box3i because Box3i isn't available in JS
        asl::Box3f myFloatBox;
        convertFrom(cx, argv[0], myFloatBox);
        asl::Box3i myVoxelBox(static_cast<int>( myFloatBox[Box3f::MIN][0]),
                              static_cast<int>( myFloatBox[Box3f::MIN][1]),
                              static_cast<int>( myFloatBox[Box3f::MIN][2]),
                              static_cast<int>( myFloatBox[Box3f::MAX][0]),
                              static_cast<int>( myFloatBox[Box3f::MAX][1]),
                              static_cast<int>( myFloatBox[Box3f::MAX][2]));
        double myThreshold;
        convertFrom(cx, argv[1], myThreshold);

        int myDownSampleRate;
        convertFrom(cx, argv[2], myDownSampleRate);

        bool myCreateNormalsFlag;
        convertFrom(cx, argv[3], myCreateNormalsFlag);

        //std::string myFilename;
        //convertFrom(cx, argv[3], myFilename);

        //bool mySaveBinaryFlag;
        //convertFrom(cx, argv[4], mySaveBinaryFlag);

        CTScan & myCTScan = myObj.getNative();
        ScenePtr myScene = myCTScan.polygonize(myVoxelBox, myThreshold, myDownSampleRate,  
            myCreateNormalsFlag, JSApp::getPackageManager());
        *rval = as_jsval(cx, myScene);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
create3DTexture(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    return Method<JSCTScan::NATIVE>::call(&JSCTScan::NATIVE::create3DTexture,cx,obj,argc,argv,rval);
}

#define DEFINE_ORIENTATION_PROP(NAME) { #NAME, PROP_ ## NAME , CTScan::NAME }

JSConstIntPropertySpec *
JSCTScan::ConstIntProperties() {

    static JSConstIntPropertySpec myProperties[] = {
        DEFINE_ORIENTATION_PROP(IDENTITY),
        DEFINE_ORIENTATION_PROP(Y2Z),
        DEFINE_ORIENTATION_PROP(X2Z),
        {0}
    };
    return myProperties;
};

JSFunctionSpec *
JSCTScan::Functions() {
    AC_DEBUG << "Registering class '"<<ClassName()<<"'"<<endl;
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {"toString",             toString,                0},
        {"loadSlices",           loadSlices,              0},
//        {"renderToImage",        renderToImage,           2},
        {"reconstructToImage",   reconstructToImage,      3},
        {"clear",                clear,                   0},
        {"verifyCompleteness",   verifyCompleteness,      0},
        {"setVoxelSize",         setVoxelSize,            1},
        {"getVoxelSize",         getVoxelSize,            0},
        {"getVoxelDimensions",   getVoxelDimensions,      0},
        {"getValueRange",        getValueRange,           0},
        {"polygonize",           polygonize,              4},
        {"countTriangles",       countTriangles,          3},
        {"create3DTexture",      create3DTexture,         2},
        {"computeHistogram",     computeHistogram,        1},
        {0}
    };
    return myFunctions;
}

JSFunctionSpec *
JSCTScan::StaticFunctions() {
    IF_REG(cerr << "Registering class '"<<ClassName()<<"'"<<endl);
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {0}
    };
    return myFunctions;
}


JSPropertySpec *
JSCTScan::Properties() {
    static JSPropertySpec myProperties[] = {
        {"default_window", PROP_default_window, JSPROP_READONLY | JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"signal_progress", PROP_signal_progress, JSPROP_READONLY | JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"length", PROP_length, JSPROP_READONLY | JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {0}
    };
    return myProperties;
}

// getproperty handling
JSBool
JSCTScan::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
        case PROP_default_window:
            *vp = as_jsval(cx, getNative().getDefaultWindow());
            return JS_TRUE;
        case PROP_length:
            *vp = as_jsval(cx, getNative().getSliceCount());
            return JS_TRUE;
        case PROP_signal_progress:
            {
                JSSignal2<void,double,Glib::ustring>::OWNERPTR mySignal( new
                        JSSignal2<void,double,Glib::ustring>::NATIVE(getNative().signal_progress()));
                *vp = jslib::as_jsval(cx, mySignal);
                return JS_TRUE;
            }
        default:
            JS_ReportError(cx,"JSCTScan::getProperty: index %d out of range", theID);
            return JS_FALSE;
    }
}

// setproperty handling
JSBool
JSCTScan::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
        case 0:
        default:
            JS_ReportError(cx,"JSCTScan::setPropertySwitch: index %d out of range", theID);
            return JS_FALSE;
    }
}

JSBool
JSCTScan::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s bad object; did you forget a 'new'?", ClassName());
        return JS_FALSE;
    }

    OWNERPTR myNewNative;
    switch (argc) {
        case 0:
            myNewNative = OWNERPTR(new CTScan());
            break;
        default:
            JS_ReportError(cx, "Constructor for %s: bad number of arguments: expected none or one "
                    "(filepath), got %d", ClassName(), argc);
            return JS_FALSE;
    }

    JSCTScan * myNewObject = new JSCTScan(myNewNative, &(*myNewNative));
    JS_SetPrivate(cx, obj, myNewObject);
    return JS_TRUE;
}

JSObject *
JSCTScan::initClass(JSContext *cx, JSObject *theGlobalObject) {
    return Base::initClass(cx, theGlobalObject, ClassName(), Constructor, Properties(), Functions(), ConstIntProperties(), 0, StaticFunctions());
}

bool convertFrom(JSContext *cx, jsval theValue, JSCTScan::OWNERPTR & theCTScan) {
    if (JSVAL_IS_OBJECT(theValue)) {
        JSObject * myArgument;
        if (JS_ValueToObject(cx, theValue, &myArgument)) {
            if (JSA_GetClass(cx,myArgument) == JSClassTraits<JSCTScan::NATIVE >::Class()) {
                theCTScan = JSClassTraits<JSCTScan::NATIVE>::getNativeOwner(cx,myArgument);
                return true;
            }
        }
    }
    return false;
}

jsval as_jsval(JSContext *cx, JSCTScan::OWNERPTR theOwner) {
    JSObject * myReturnObject = JSCTScan::Construct(cx, theOwner, &(*theOwner));
    return OBJECT_TO_JSVAL(myReturnObject);
}

jsval as_jsval(JSContext *cx, JSCTScan::OWNERPTR theOwner, JSCTScan::NATIVE * theCTScan) {
    JSObject * myObject = JSCTScan::Construct(cx, theOwner, theCTScan);
    return OBJECT_TO_JSVAL(myObject);
}

}
