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
#include "Client.h"
#include "../NetAsync.h"

#include <string>
#include <y60/jsbase/JScppUtils.h>
#include <y60/jsbase/JSBlock.h>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include <netsrc/spidermonkey/jsapi.h>

using namespace std;
using namespace asl;
using namespace jslib;

namespace y60 {
namespace async {
namespace http {
namespace curl {

    Client::Client(JSContext * cx, JSObject * theOpts) :
        _curlHandle(0),
        _jsContext(cx),
        _jsOptsObject(theOpts),
        _myErrorBuffer(CURL_ERROR_SIZE, '\0'),
        _privateResponseBuffer(new Block()),
        _myResponseBlock(new Block()),
        _continueFlag(true)
    {
        _curlHandle = curl_easy_init();
        AC_DEBUG << "curl init " << curl_version();

        CURLcode myStatus;
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_ERRORBUFFER, asl::begin_ptr(_myErrorBuffer));
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_PRIVATE, this);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_WRITEFUNCTION, &Client::_writeFunction);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_WRITEDATA, this);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_OPENSOCKETFUNCTION, &Client::_openSocket);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_OPENSOCKETDATA, this);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);

        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_CLOSESOCKETFUNCTION, &Client::_closeSocket);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        myStatus = curl_easy_setopt(_curlHandle, CURLOPT_CLOSESOCKETDATA, this);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);

        ostringstream s;
        std::string url;
        setCurlOption<std::string>(_jsOptsObject, "url", CURLOPT_URL, &url);
        s << "HttpClient::" << hex << this;
        s << " " << url;
        AC_DEBUG << "creating client " << this << " with _jsOptsObject " << _jsOptsObject;
        debugIdentifier = s.str();

        setCurlOption<bool>(_jsOptsObject, "verbose", CURLOPT_VERBOSE, 0);
        setCurlOption<long>(_jsOptsObject, "connecttimeout", CURLOPT_CONNECTTIMEOUT, 0);

        if(!JS_AddNamedRoot(_jsContext, &_jsOptsObject, debugIdentifier.c_str())) {
            AC_WARNING << "failed to root request object!";
        }
    
    }

    void 
    Client::performSync() {
        CURLcode myStatus = curl_easy_perform(_curlHandle);
        onDone(myStatus);
    }

    void
    Client::performAsync() {
        AC_DEBUG << "starting request " << this;
        asl::Ptr<NetAsync> parentPlugin = dynamic_cast_Ptr<NetAsync>(Singleton<PlugInManager>::get().getPlugIn(NetAsync::PluginName));
        parentPlugin->getCurlAdapater().addClient(shared_from_this());
    }

    void 
    Client::setWrapper(JSObject * theWrapper) {
        _jsWrapper = theWrapper;
        // add root to prevent garbage collection of client and its callbacks 
        if(!JS_AddNamedRoot(_jsContext, &_jsWrapper, debugIdentifier.c_str())) {
            AC_WARNING << "failed to root request object!";
        }
    }

    Client::~Client()
    {
        AC_DEBUG << "~Client " << this;
        CURLcode myStatus = curl_easy_setopt(_curlHandle, CURLOPT_OPENSOCKETDATA, 0);
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
    }
   
    void
    Client::checkCurlStatus(CURLcode theStatusCode, const std::string & theWhere) {
        if (theStatusCode != CURLE_OK) {
            throw asl::Exception(string(asl::begin_ptr(_myErrorBuffer)), theWhere);
        }
    };

    std::string
    Client::getResponseString() const {
        return std::string(_myResponseBlock->strbegin(), _myResponseBlock->strend());
    };

    asl::Ptr<asl::Block> 
    Client::getResponseBlock() const {
        return _myResponseBlock;
    };
    void 
    Client::onProgress() {
        bool newDataReceived = false;
        {
            ScopeLocker L(_lockResponseBuffer, true);
            if (_privateResponseBuffer->size() > 0) {
                _myResponseBlock->append(*_privateResponseBuffer);
                _privateResponseBuffer->resize(0);
                newDataReceived = true;
            }
        }
        if (newDataReceived && hasCallback("progress")) {
            AC_TRACE << "calling onProgress for " << this;
            jsval argv[1], rval;
            argv[0] = as_jsval(_jsContext, _myResponseBlock);
            JSBool ok = JSA_CallFunctionName(_jsContext, _jsOptsObject, "progress", 1, argv, &rval);
            if (ok) {
                if (!convertFrom(_jsContext, rval, _continueFlag)) {
                    std::cerr << "#ERROR: HttpClient: progress callback returned a bad result (not a bool)" << std::endl;
                    _continueFlag = true;
                }
            }
        }
    };

JSBool
JSA_CallFunctionName(JSContext * cx, JSObject * theThisObject, JSObject * theObject, const char * theName, uintN argc, jsval argv[], jsval *rval) {
    jsval myValue;
    if (JS_GetProperty(cx, theObject, theName, &myValue)) {
        if (JS_TypeOfValue(cx, myValue) != JSTYPE_FUNCTION) {
            AC_WARNING << "Property '" << theName << "' is not a function: type=" << JS_TypeOfValue(cx, myValue);
            return false;
        }
    }
    try {
        AC_DEBUG << "cx:" << cx << ", this:" << theThisObject << ", obj:" << theObject << ", theName:" << theName << ", argc:" << argc << ", argv:" << argv << ", rval:" << rval;
        JSBool ok = JS_CallFunctionValue(cx, theThisObject, myValue, argc, argv, rval);
        if (!ok) {
            AC_DEBUG << "Exception while calling js function '" << theName << "'" << endl;
        }
        return ok;
    } HANDLE_CPP_EXCEPTION;
};

    void
    Client::onDone(CURLcode result) {
        {
            ScopeLocker L(_lockResponseBuffer, true);
            _myResponseBlock->append(*_privateResponseBuffer);
            _privateResponseBuffer->resize(0);
        }

        AC_DEBUG << "onDone. CURLcode is " << result << " for " << this;
        AC_DEBUG << "error string:" << std::string(asl::begin_ptr(_myErrorBuffer));
        if (result == CURLE_OK) {
            if (hasCallback("success")) {
                AC_DEBUG << "calling success";
                jsval argv[1], rval;
                /*JSBool ok =*/ JSA_CallFunctionName(_jsContext, _jsWrapper, _jsOptsObject, "success", 0, argv, &rval);
                AC_DEBUG << "called success";
            };
        } else {
            if (hasCallback("error")) {
                AC_DEBUG << "calling error";
                jsval argv[2], rval;
                argv[0] = as_jsval(_jsContext, result);
                argv[1] = as_jsval(_jsContext, std::string(asl::begin_ptr(_myErrorBuffer)));
                /*JSBool ok =*/ JSA_CallFunctionName(_jsContext, _jsWrapper, _jsOptsObject, "error", 2, argv, &rval);
            };
        }
        AC_DEBUG << "freeing root for " << debugIdentifier;
        JS_RemoveRoot(_jsContext, &_jsOptsObject);
        JS_RemoveRoot(_jsContext, &_jsWrapper);
    }

    bool
    Client::hasCallback(const char * theName) {
        jsval myValue;
        if (JS_GetProperty(_jsContext, _jsOptsObject, theName, &myValue)) {
            if (JS_TypeOfValue(_jsContext, myValue) == JSTYPE_FUNCTION) {
                return true;
            }
        }
        return false;
    }

    size_t 
    Client::writeFunction(const unsigned char *ptr, size_t size) {
        // NOTE: this will be called from one of io_service's threads
        ScopeLocker L(_lockResponseBuffer, true);
        _privateResponseBuffer->append(ptr, size);
        return _continueFlag ? size : 0;
    };

    curl_socket_t 
    Client::openSocket(curlsocktype purpose, struct curl_sockaddr *addr) {
        AC_DEBUG << "curl requesting open socket";
        asl::Ptr<NetAsync> parentPlugin = dynamic_cast_Ptr<NetAsync>(Singleton<PlugInManager>::get().getPlugIn(NetAsync::PluginName));
        return parentPlugin->getCurlAdapater().openSocket();
    };
    
    int 
    Client::_closeSocket(Client *self, curl_socket_t item) {
        AC_DEBUG << "closing socket " << item;
        SocketPtr s = SocketAdapter::find(item);
        if (s) {
            s->close();
        }
        return 0;
    };
    
    template<>
    bool
    Client::setCurlOption<std::string>(JSObject* opts, std::string theProperty, CURLoption theCurlOption, std::string * theValue) {
        jsval propValue;
        std::string nativeValue;

        JS_GetProperty(_jsContext, opts, theProperty.c_str(), &propValue);
        if (JSVAL_IS_VOID(propValue)) {
            return false;
        }
        if (!jslib::convertFrom(_jsContext, propValue, nativeValue)) {
            JS_ReportError(_jsContext, "Type mismatch on %s", theProperty.c_str());
        }
        CURLcode myStatus = curl_easy_setopt(_curlHandle, theCurlOption, nativeValue.c_str());
        checkCurlStatus(myStatus, PLUS_FILE_LINE);
        AC_DEBUG << "set string option " << theProperty << " = '" << nativeValue << "'";
        if (theValue) {
            *theValue = nativeValue;
        }
        return true;
    };
     
}
}
}
}
