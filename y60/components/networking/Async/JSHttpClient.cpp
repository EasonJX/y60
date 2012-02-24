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

#include "JSHttpClient.h"

#include <y60/jsbase/JSWrapper.impl>
#include <y60/jsbase/JSBlock.h>
#include <y60/jsbase/JSNode.h>

#include <iostream>

#include <netsrc/spidermonkey/jsfun.h>

using namespace std;
using namespace asl;
using namespace jslib;
using namespace y60;


const unsigned READ_BUFFER_SIZE = 20000;

#define DB(x) // x

static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
/*    DOC_BEGIN("Returns information on socket connection."); DOC_END;
    const inet::Socket & mySocket = JSHttpClient::getJSWrapper(cx,obj).getNative();
    std::string myStringRep = string("Socket [local ip: ") +
        as_dotted_address(mySocket.getLocalAddress()) + ", port: " +
        as_string(mySocket.getLocalPort()) + " | remote ip: " +
        as_dotted_address(mySocket.getRemoteAddress()) + ", port: " +
        as_string(mySocket.getRemotePort()) + "]";
    *rval = as_jsval(cx, myStringRep);*/
    return JS_TRUE;
}

JSHttpClient::~JSHttpClient() {
}

JSFunctionSpec *
JSHttpClient::Functions() {
    IF_REG(cerr << "Registering class '"<<ClassName()<<"'"<<endl);
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {"toString",             toString,                0},
        {0}
    };
    return myFunctions;
}

JSPropertySpec *
JSHttpClient::Properties() {
    static JSPropertySpec myProperties[] = {
        {"responseString", PROP_responseString, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED},
        {"responseBlock", PROP_responseBlock, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED},
        {0}
    };
    return myProperties;
}

JSPropertySpec *
JSHttpClient::StaticProperties() {
    static JSPropertySpec myProperties[] = {
        {0}
    };
    return myProperties;
}

JSFunctionSpec *
JSHttpClient::StaticFunctions() {
    static JSFunctionSpec myFunctions[] = {
        {0}
    };
    return myFunctions;
}

JSConstIntPropertySpec *
JSHttpClient::ConstIntProperties() {
    static JSConstIntPropertySpec myProperties[] = {
        {0}
    };
    return myProperties;
};

// getproperty handling
JSBool
JSHttpClient::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
            case PROP_responseString:
                *vp = as_jsval(cx, getNative().getResponseString());
                return JS_TRUE;
            case PROP_responseBlock:
                *vp = as_jsval(cx, getNative().getResponseBlock());
                return JS_TRUE;
            default:
                JS_ReportError(cx,"JSRequestWrapper::getProperty: index %d out of range", theID);
                return JS_FALSE;
    }
    return JS_TRUE;
}

// setproperty handling
JSBool
JSHttpClient::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    return JS_TRUE;
}


JSBool
JSHttpClient::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Creates a HttpClient.");
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s bad object; did you forget a 'new'?", ClassName());
        return JS_FALSE;
    }

    JSObject* optsObject;
    if (!JSVAL_IS_OBJECT(argv[0])) {
        JS_ReportError(cx, "HttpClient::Construct: argument #1 is not an object");
        return JS_FALSE;
    } 
    JS_ValueToObject(cx,argv[0],&optsObject);
    
    JSHttpClient * myNewObject = 0;
    OWNERPTR myHttpClient = OWNERPTR(new async::http::Client(cx, optsObject));
    myNewObject = new JSHttpClient(myHttpClient, myHttpClient.get());
    JS_SetPrivate(cx, obj, myNewObject);
    // now stick the opts object to the JSHttpClient wrapper so it's not GC'ed
    jsval optsValue = OBJECT_TO_JSVAL(optsObject);
    JS_SetProperty(cx, obj, "_opts", &optsValue);
    return JS_TRUE;
}


JSObject *
JSHttpClient::initClass(JSContext *cx, JSObject *theGlobalObject) {    
    JSObject *myClass = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, Properties(), Functions(), ConstIntProperties());
    DOC_MODULE_CREATE("HttpClient", JSHttpClient);
    return myClass;
}

bool convertFrom(JSContext *cx, jsval theValue, JSHttpClient::OWNERPTR & theNativePtr) {
    if (JSVAL_IS_OBJECT(theValue)) {
        JSObject * myArgument;
        if (JS_ValueToObject(cx, theValue, &myArgument)) {
            if (JSA_GetClass(cx,myArgument) == JSClassTraits<JSHttpClient::NATIVE>::Class()) {
                theNativePtr = JSClassTraits<JSHttpClient::NATIVE>::getNativeOwner(cx,myArgument);
                return true;
            }
        }
    }
    return false;
}

jsval as_jsval(JSContext *cx, JSHttpClient::OWNERPTR theOwner) {
    JSObject * myReturnObject = JSHttpClient::Construct(cx, theOwner, theOwner.get());
    return OBJECT_TO_JSVAL(myReturnObject);
}


jsval as_jsval(JSContext *cx, JSHttpClient::OWNERPTR theOwner, JSHttpClient::NATIVE * theSerial) {
    JSObject * myObject = JSHttpClient::Construct(cx, theOwner, theSerial);
    return OBJECT_TO_JSVAL(myObject);
}
