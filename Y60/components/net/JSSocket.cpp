//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "JSSocket.h"

#include <asl/net.h>
#include <asl/SocketException.h>
#include <asl/UDPConnection.h>
#include <asl/TCPClientSocket.h>
#include <asl/os_functions.h>
#include <asl/net_functions.h>
#include <y60/JSBlock.h>

#include <iostream>

using namespace std;
using namespace asl;
using namespace jslib;

const unsigned SOCKET_TYPE_UDP       = 0;
const unsigned SOCKET_TYPE_TCPCLIENT = 1;

const unsigned READ_BUFFER_SIZE = 20000;

#define DB(x) // x

static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Returns information on socket connection."); DOC_END;
    const inet::Socket & mySocket = JSSocket::getJSWrapper(cx,obj).getNative();
    std::string myStringRep = string("Socket [local ip: ") +
        as_dotted_address(mySocket.getLocalAddress()) + ", port: " +
        as_string(mySocket.getLocalPort()) + " | remote ip: " +
        as_dotted_address(mySocket.getRemoteAddress()) + ", port: " +
        as_string(mySocket.getRemotePort()) + "]";
    JSString * myString = JS_NewStringCopyN(cx,myStringRep.c_str(),myStringRep.size());
    *rval = STRING_TO_JSVAL(myString);
    return JS_TRUE;
}

static JSBool
close(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Close connection."); DOC_END;
    return Method<JSSocket::NATIVE>::call(&JSSocket::NATIVE::close,cx,obj,argc,argv,rval);
}

static JSBool
read(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    string docStr = "Read data from connected socket. (max. bytes " + asl::as_string(READ_BUFFER_SIZE) + ")";
    DOC_BEGIN(docStr);
    DOC_RVAL("number of bytes read.", DOC_TYPE_INTEGER);
    DOC_END;
    try {
        if (argc != 0) {
            JS_ReportError(cx, "JSSocket::read(): Wrong number of arguments, expected none, got %d.", argc);
            return JS_FALSE;
        }

        char myBuffer[READ_BUFFER_SIZE];
        unsigned myBytesRead = JSSocket::getJSWrapper(cx,obj).openNative().receive(myBuffer, READ_BUFFER_SIZE);
        JSSocket::getJSWrapper(cx,obj).closeNative();

        DB(cerr << "Socket::read(): " << string(myBuffer, myBytesRead) << endl;)

        JSString * myString = JS_NewStringCopyN(cx, myBuffer, myBytesRead);
        *rval = STRING_TO_JSVAL(myString);
        return JS_TRUE;
    } catch(inet::SocketException &) {
        // TODO, must be reworked once we can throw exception into javascript
        *rval = -1;
        return JS_TRUE;
    }
}

static JSBool
write(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Write data to connected socket.");
    DOC_PARAM("theBlock", "A binary block to send", DOC_TYPE_OBJECT);
    DOC_RESET;
    DOC_PARAM("theString", "The string to write. (make sure there are no zero bytes inside)", DOC_TYPE_STRING);
    DOC_RESET;
    DOC_PARAM("theArrayOfUnsignedByte", "An array of numbers less than 256 (if you need to send zero bytes)", DOC_TYPE_ARRAY);
    DOC_RVAL("Number of bytes written.", DOC_TYPE_INTEGER);
    DOC_END;
    try {
        if (argc != 1) {
            JS_ReportError(cx, "JSSocket::write(): Wrong number of arguments, "
                               "expected one (Bytes to write), got %d.", argc);
            return JS_FALSE;
        }
        if (JSVAL_IS_VOID(argv[0])) {
            JS_ReportError(cx, "JSSocket::write(): Argument #0 is undefined");
            return JS_FALSE;
        }

        unsigned myBytesWritten = 0;
        if (JSBlock::matchesClassOf(cx, argv[0])) {
            asl::Block * myBlock;
            if (!convertFrom(cx, argv[0], myBlock)) {
                JS_ReportError(cx, "JSSocket::write(): Could not convert block argument");
                return JS_FALSE;
            }

            myBytesWritten = JSSocket::getJSWrapper(cx,obj).openNative().send(myBlock->begin(), myBlock->size());
            JSSocket::getJSWrapper(cx,obj).closeNative();
        } else {
            string myString;
            if (JSVAL_IS_STRING(argv[0])) {
                if (!convertFrom(cx, argv[0], myString)) {
                    JS_ReportError(cx, "JSSocket::write(): Argument #1 must be an string or block or an array of unsigned char (Bytes to write)");
                    return JS_FALSE;
                }
            } else {
                if (JSA_charArrayToString(cx, argv, myString) == JS_FALSE) {
                    return JS_FALSE;
                }

                if (myString.empty()) {
                    JS_ReportError(cx, "JSSocket::write(): Argument #1 must be an string or "
                        "an array of unsigned char or block (Bytes to write)");
                    return JS_FALSE;
                }
            }

            myBytesWritten = JSSocket::getJSWrapper(cx,obj).openNative().send(myString.c_str(), myString.size());
            JSSocket::getJSWrapper(cx,obj).closeNative();
        }

        *rval = as_jsval(cx, myBytesWritten);

        return JS_TRUE;
    } catch(inet::SocketException &) {
        // TODO, must be reworked once we can throw exception into javascript
        *rval = -1;
        return JS_TRUE;
    }
}

static JSBool
peek(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Peek for available bytes at connected socket");
    DOC_PARAM("theBufferSize", "Maximum number of bytes to be peeked at.", DOC_TYPE_INTEGER);
    DOC_RVAL("The number of bytes available", DOC_TYPE_INTEGER);
    DOC_END;
    return Method<JSSocket::NATIVE>::call(&JSSocket::NATIVE::peek,cx,obj,argc,argv,rval);
}

static JSBool
connect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Connect to socket.");
    DOC_PARAM("theRemoteIPAddress", "IP-address or hostname to connect to", DOC_TYPE_STRING);
    DOC_PARAM("theRemotePort", "Port number to connect to", DOC_TYPE_INTEGER);
    DOC_RVAL("True if succesful", DOC_TYPE_BOOLEAN);
    DOC_END;
    try {
        if (argc != 2) {
            JS_ReportError(cx, "Socket::connect: bad number of arguments: expected 2 "
                "(remote port, remote ip adress), got %d", argc);
            return JS_FALSE;
        }

        if (JSVAL_IS_VOID(argv[0])) {
            JS_ReportError(cx,"Socket::connect: bad argument #1 (undefined)");
            return JS_FALSE;
        }
        if (JSVAL_IS_VOID(argv[1])) {
            JS_ReportError(cx,"Socket::connect: bad argument #2 (undefined)");
            return JS_FALSE;
        }

        string myRemoteHostAddressString;
        if (!convertFrom(cx, argv[0], myRemoteHostAddressString)) {
            JS_ReportError(cx, "Socket::connect: argument #1 must be a string (remote ip address)");
            return JS_FALSE;
        }

        unsigned short myRemotePort;
        if (!convertFrom(cx, argv[1], myRemotePort)) {
            JS_ReportError(cx, "Socket::connect: argument #2 must be an unsigned short (remote port)");
            return JS_FALSE;
        }

        unsigned long myRemoteHostAddress = asl::hostaddress(myRemoteHostAddressString);
        JSClassTraits<JSSocket::NATIVE>::ScopedNativeRef myScopedSocket(cx, obj);
        inet::UDPConnection * myUDPSocket = dynamic_cast<inet::UDPConnection *>(&myScopedSocket.getNative());
        if (myUDPSocket) {
            myUDPSocket->connect(myRemoteHostAddress, myRemotePort);
            //myUDPSocket->setRemoteAddr(myRemoteHostAddress, myRemotePort); // Done in connect
            return JS_TRUE;
        }

        inet::TCPClientSocket * myTCPClientSocket = dynamic_cast<inet::TCPClientSocket *>(&myScopedSocket.getNative());
        if (!myTCPClientSocket) {
            return JS_FALSE;
        } else {
            myTCPClientSocket->setRemoteAddr(myRemoteHostAddress, myRemotePort);
            myTCPClientSocket->connect();
            return JS_TRUE;
        }
    } catch(inet::SocketException &) {
        // TODO, must be reworked once we can throw exception into javascript
        *rval = -1;
        return JS_TRUE;
    }
}

static JSBool
setBlockingMode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Set blocking mode on connection");
    DOC_PARAM("isBlocking", "If false, a read call on the socket will only return if a packet has been received", DOC_TYPE_BOOLEAN);
    DOC_END;
    return Method<JSSocket::NATIVE>::call(&JSSocket::NATIVE::setBlockingMode,cx,obj,argc,argv,rval);
}

JSSocket::~JSSocket() {
    inet::terminateSockets();
}

JSFunctionSpec *
JSSocket::Functions() {
    IF_REG(cerr << "Registering class '"<<ClassName()<<"'"<<endl);
    static JSFunctionSpec myFunctions[] = {
        // name                  native                   nargs
        {"toString",             toString,                0},
        {"read",                 read,                    0},
        {"write",                write,                   1},
        {"peek",                 peek,                    1},
        {"close",                close,                   0},
        {"connect",              connect,                 2},
        {"setBlockingMode",      setBlockingMode,         1},
        {0}
    };
    return myFunctions;
}

JSPropertySpec *
JSSocket::Properties() {
    static JSPropertySpec myProperties[] = {
        {"isValid", PROP_isValid, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"sendBufferSize", PROP_sendBufferSize, JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"receiveBufferSize", PROP_receiveBufferSize, JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {0}
    };
    return myProperties;
}

JSPropertySpec *
JSSocket::StaticProperties() {
    static JSPropertySpec myProperties[] = {
        {0}
    };
    return myProperties;
}

JSFunctionSpec *
JSSocket::StaticFunctions() {
    static JSFunctionSpec myFunctions[] = {
        {0}
    };
    return myFunctions;
}

// getproperty handling
JSBool
JSSocket::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
        case PROP_isValid:
            *vp = as_jsval(cx, getNative().isValid());
            return JS_TRUE;
        case PROP_sendBufferSize:
            *vp = as_jsval(cx, getNative().getSendBufferSize());
            return JS_TRUE;
        case PROP_receiveBufferSize:
            *vp = as_jsval(cx, getNative().getReceiveBufferSize());
            return JS_TRUE;
        default:
            JS_ReportError(cx,"JSSocket::getProperty: index %d out of range", theID);
            return JS_FALSE;
    }
}

// setproperty handling
JSBool
JSSocket::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    jsval dummy;
    switch (theID) {
        case PROP_isValid:
            //return Method<NATIVE>::call(&NATIVE::isOpen, cx, obj, 1, vp, &dummy);
            return JS_FALSE;
        case PROP_sendBufferSize:
            return Method<NATIVE>::call(&NATIVE::setSendBufferSize, cx, obj, 1, vp, &dummy);
            return JS_TRUE;
        case PROP_receiveBufferSize:
            return Method<NATIVE>::call(&NATIVE::setReceiveBufferSize, cx, obj, 1, vp, &dummy);
            return JS_TRUE;
        default:
            JS_ReportError(cx,"JSSocket::setPropertySwitch: index %d out of range", theID);
            return JS_FALSE;
    }
}

JSBool
JSSocket::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Creates a Socket. No connection is established.");
    DOC_PARAM("theSocketType", "UDP or TCPCLIENT", DOC_TYPE_STRING);
    DOC_PARAM("thePort", "The local port the socket is listening on", DOC_TYPE_INTEGER);
    DOC_PARAM_OPT("theLocalIPAddress", "The local ip-address or hostname the socket is listening on", DOC_TYPE_STRING, asl::localhostname());
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s bad object; did you forget a 'new'?", ClassName());
        return JS_FALSE;
    }
    static bool myInitializedFlag = false;
    if (!myInitializedFlag) {
        inet::initSockets();
        myInitializedFlag = true;
    }

    JSSocket * myNewObject = 0;
    if (argc == 0 ) {
        OWNERPTR mySocket = OWNERPTR(new inet::Socket(0,0));
        myNewObject = new JSSocket(mySocket, &(*mySocket));
    }
    if (argc == 1 || argc > 3) {
        JS_ReportError(cx, "Constructor for %s: bad number of arguments: expected 2 or 3 "
            "(sockettype, local port [, local ip adress]), got %d", ClassName(), argc);
        return JS_FALSE;
    }

    if (argc == 2 || argc ==  3) {



        if (JSVAL_IS_VOID(argv[0])) {
            JS_ReportError(cx,"JSSocket::Constructor: bad argument #1 (undefined)");
            return JS_FALSE;
        }
        if (JSVAL_IS_VOID(argv[1])) {
            JS_ReportError(cx,"JSSocket::Constructor: bad argument #2 (undefined)");
            return JS_FALSE;
        }

        unsigned short mySocketType;
        if (!convertFrom(cx, argv[0], mySocketType) || mySocketType > 2) {
            JS_ReportError(cx, "JSSocket::Constructor: argument #1 must be [UDP | TCPCLIENT]");
            return JS_FALSE;
        }

        unsigned short myLocalPort;
        if (!convertFrom(cx, argv[1], myLocalPort)) {
            JS_ReportError(cx, "JSSocket::Constructor: argument #2 must be an unsigned short (local port)");
            return JS_FALSE;
        }

        string myLocalHostAddressString;
        if (argc > 2) {
            if (JSVAL_IS_VOID(argv[2])) {
                JS_ReportError(cx,"JSSocket::Constructor: bad argument #3 (undefined)");
                return JS_FALSE;
            }

            if (!convertFrom(cx, argv[2], myLocalHostAddressString)) {
                JS_ReportError(cx, "JSSocket::Constructor: argument #3 must be a string (local ip address)");
                return JS_FALSE;
            }
        } else {
            myLocalHostAddressString = asl::localhostname();
        }

        unsigned long myLocalHostAddress = asl::hostaddress(myLocalHostAddressString);
        try {
            if (mySocketType == SOCKET_TYPE_UDP) {
                OWNERPTR myNewSocket = OWNERPTR(new inet::UDPConnection(myLocalHostAddress, myLocalPort));
                myNewObject = new JSSocket(myNewSocket, &(*myNewSocket));
            } else if (mySocketType == SOCKET_TYPE_TCPCLIENT) {
                OWNERPTR myNewSocket = OWNERPTR(new inet::TCPClientSocket());

                myNewObject = new JSSocket(myNewSocket, &(*myNewSocket));
            }
        } HANDLE_CPP_EXCEPTION;
    }

    if (myNewObject) {
        JS_SetPrivate(cx, obj, myNewObject);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSSocket::Constructor: bad parameters");
    return JS_FALSE;
}

JSConstIntPropertySpec *
JSSocket::ConstIntProperties() {

    static JSConstIntPropertySpec myProperties[] = {
            "UDP",               PROP_UDP,               SOCKET_TYPE_UDP,
            "TCPCLIENT",         PROP_TCPCLIENT,         SOCKET_TYPE_TCPCLIENT,
        {0}
    };
    return myProperties;
};

JSObject *
JSSocket::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject *myClass = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, Properties(), Functions(), ConstIntProperties());
    DOC_MODULE_CREATE("Components", JSSocket);
    return myClass;
}

bool convertFrom(JSContext *cx, jsval theValue, JSSocket::OWNERPTR & theNativePtr) {
    if (JSVAL_IS_OBJECT(theValue)) {
        JSObject * myArgument;
        if (JS_ValueToObject(cx, theValue, &myArgument)) {
            if (JSA_GetClass(cx,myArgument) == JSClassTraits<JSSocket::NATIVE>::Class()) {
                theNativePtr = JSClassTraits<JSSocket::NATIVE>::getNativeOwner(cx,myArgument);
                return true;
            }
        }
    }
    return false;
}


bool convertFrom(JSContext *cx, jsval theValue, JSSocket::NATIVE & theNative) {
    if (JSVAL_IS_OBJECT(theValue)) {
        JSObject * myArgument;
        if (JS_ValueToObject(cx, theValue, &myArgument)) {
            if (JSA_GetClass(cx,myArgument) == JSClassTraits<JSSocket::NATIVE >::Class()) {
                theNative = JSClassTraits<JSSocket::NATIVE>::getNativeRef(cx,myArgument);
                return true;
            }
        }
    }
    return false;
}

jsval as_jsval(JSContext *cx, JSSocket::OWNERPTR theOwner) {
    JSObject * myReturnObject = JSSocket::Construct(cx, theOwner, &(*theOwner));
    return OBJECT_TO_JSVAL(myReturnObject);
}


jsval as_jsval(JSContext *cx, JSSocket::OWNERPTR theOwner, JSSocket::NATIVE * theSerial) {
    JSObject * myObject = JSSocket::Construct(cx, theOwner, theSerial);
    return OBJECT_TO_JSVAL(myObject);
}
