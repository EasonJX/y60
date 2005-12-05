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
//   $RCSfile: JSNamedNodeMap.cpp,v $
//   $Author: pavel $
//   $Revision: 1.3 $
//   $Date: 2005/04/24 00:41:19 $
//
//
//=============================================================================

#include "JSNodeList.h"
#include "JSNamedNodeMap.h"
#include <iostream>

using namespace std;

namespace jslib {

// TODO: add namedItem getter and setter functions
JSFunctionSpec *
JSNamedNodeMap::Functions() {
    AC_DEBUG << "Registering class '"<<ClassName()<<"'"<<endl;
    static JSFunctionSpec myFunctions[] = {
        /* name         native          nargs    */
        {"item",             JSNodeListBase<dom::NodeList>::item,            1}, // gcc does not allow function pointers to inherited functions
        {"appendNode",       JSNodeListBase<dom::NodeList>::appendNode,      1},
        //{"replaceChild",     replaceChild,    1},
//        {"setAllNodeValues",  JSNodeListBase<dom::NodeList>::setAllNodeValues,    1},
        {0}
    };
    return myFunctions;
}

enum PropertyNumbers {PROP_length = -100};

static JSPropertySpec * Properties() {
    static JSPropertySpec myProperties[] = {
        {"length", PROP_length, JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT|JSPROP_SHARED},   // readonly attribute unsigned long
        {0}
    };
    return myProperties;
}

JSConstIntPropertySpec *
JSNamedNodeMap::ConstIntProperties() {
    static JSConstIntPropertySpec myProperties[] = {{0}};
    return myProperties;
}

JSPropertySpec *
JSNamedNodeMap::StaticProperties() {
    static JSPropertySpec myProperties[] = {{0}};
    return myProperties;
}

JSFunctionSpec *
JSNamedNodeMap::StaticFunctions() {
    static JSFunctionSpec myFunctions[] = {{0}};
    return myFunctions;
}

// getproperty handling

JSBool
JSNamedNodeMap::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
            case PROP_length:
                *vp = as_jsval(cx, getNative().length());
                return JS_TRUE;
            default:
                JS_ReportError(cx,"JSNamedNodeMap::getProperty: index %d out of range", theID);
                return JS_FALSE;
    }
};
JSBool JSNamedNodeMap::getPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    *vp = as_jsval(cx, getNative().item(theIndex));
    return JS_TRUE;
};

// setproperty handling
JSBool JSNamedNodeMap::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JS_ReportError(cx,"JSNamedNodeMap::setPropertySwitch: index %d out of range", theID);
    return JS_FALSE;
};
JSBool JSNamedNodeMap::setPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    dom::NodePtr myArgNode1 = JSNode::getNodePtr(cx, *vp);
    openNative().item(theIndex) = myArgNode1;
    closeNative();
    return JS_TRUE;
};

JSBool
JSNamedNodeMap::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Constructs a new NamedNodeMap.");
    DOC_END;
    IF_NOISY2(AC_TRACE << "Constructor argc =" << argc << endl);
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }
    JSNamedNodeMap * myNewObject = 0;
    dom::NodePtr myNewNode = dom::NodePtr(new dom::Node);
    if (argc == 0) {
        myNewObject=new JSNamedNodeMap(myNewNode, &(myNewNode->attributes()));
    } else {
        JS_ReportError(cx,"Constructor for %s: superflous argument ignored",ClassName());
        myNewObject=new JSNamedNodeMap(myNewNode, &(myNewNode->attributes()));
    }
    if (myNewObject) {
        JS_SetPrivate(cx,obj,myNewObject);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSVector::Constructor: bad parameters");
    return JS_FALSE;
}

JSObject *
JSNamedNodeMap::Construct(JSContext *cx, dom::NodePtr theNode, dom::NamedNodeMap * theList) {
    return Base::Construct(cx, theNode, theList);
}

}
