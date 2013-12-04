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

*/

/*jslint white : false*/
/*globals use plug UnitTest millisec Socket HttpServer ENSURE UnitTestSuite
          print exit RequestManager Request*/

use("UnitTest.js");

plug("Network");
plug("NetAsync");

var TIMEOUT = 1000;

function HttpServerUnitTest() {
    this.Constructor(this, "HttpServerUnitTest");
}

HttpServerUnitTest.prototype.Constructor = function (obj, theName) {

    UnitTest.prototype.Constructor(obj, theName);

    obj.run = function () {

        obj.callback_answer = "callback";

        var myObj = {
            test : function (theMethod, theBody) {
                return obj.callback_answer;
            },
            fallback : function (theMethod, theBody, thePath) {
                return thePath;
            },
            detailed_response : function (theMethod, theBody, thePath) {
                return [202, { "X-PRODUCED-BY" : "Y60" }, "Hello"];
            },
            detailed_response_2 : function (theMethod, theBody, thePath) {
                    return [201, "I am Content"];
            },
            detailed_response_3 : function (theMethod, theBody, thePath) {
                    return [];
            },
            detailed_response_4 : function (theMethod, theBody, thePath) {
                    return;
            }

        };

        obj.myServer = new Async.HttpServer();

        obj.myServer.registerCallback("/test", myObj, myObj.test);
        obj.myServer.registerCallback("/foo", myObj, myObj.detailed_response);
        obj.myServer.registerCallback("/bar", myObj, myObj.detailed_response_2);
        obj.myServer.registerCallback("/krokodil", myObj, myObj.detailed_response_3);
        obj.myServer.registerCallback("/versicherung", myObj, myObj.detailed_response_4);
        obj.myServer.registerCallback("*", myObj, myObj.fallback);

        obj.myServer.start("0.0.0.0", "4042");

        // detailed responses
        
        // path: bar
        var myRequestManager  = new RequestManager();
        var myRequest = new Request("http://localhost:4042/bar");
        myRequest.onDone = function () {
            obj.testResponse = this;
            ENSURE("obj.testResponse.responseCode == '201'");
            ENSURE_EQUAL('I am Content', obj.testResponse.responseString);
            ENSURE("obj.testResponse.getResponseHeader('Content-Type') == 'text/plain'");
            ENSURE("obj.testResponse.getResponseHeader('Content-Length') == obj.testResponse.responseString.length");
        };
        myRequest.get();

        myRequestManager.performRequest(myRequest);
        while (myRequestManager.activeCount > 0) {
            Async.onFrame();
            myRequestManager.handleRequests();
        }

        // path: foo
        myRequest = new Request("http://localhost:4042/foo");
        myRequest.onError = function () {
            obj.testResponse = this;
            ENSURE_EQUAL("202", obj.testResponse.responseCode);
            ENSURE("obj.testResponse.getResponseHeader('Content-Type') == 'text/plain'");
            ENSURE("obj.testResponse.getResponseHeader('X-PRODUCED-BY') == 'Y60'");
        };
        // myRequest.verbose = true;
        myRequest.get();
        
        myRequestManager.performRequest(myRequest);
        while (myRequestManager.activeCount > 0) {
            Async.onFrame();
            myRequestManager.handleRequests();
        }

        // path: krokodil
        myRequest = new Request("http://localhost:4042/krokodil");
        myRequest.onDone = function () {
            obj.testResponse = this;
            ENSURE("obj.testResponse.responseString == ''");
            ENSURE("obj.testResponse.responseCode == '200'");
            ENSURE("obj.testResponse.getResponseHeader('Content-Type') == 'text/plain'");
        };
        myRequest.get();
        
        myRequestManager.performRequest(myRequest);
        while (myRequestManager.activeCount > 0) {
            myRequestManager.handleRequests();
            Async.onFrame();
        }
        
        // path: versicherung
        myRequest = new Request("http://localhost:4042/versicherung");
        myRequest.onDone = function () {
            obj.testResponse = this;
            ENSURE("obj.testResponse.responseString == ''");
            ENSURE("obj.testResponse.responseCode == '204'");
            ENSURE("obj.testResponse.getResponseHeader('Content-Type') == 'text/plain'");
        };
        myRequest.get();
        
        myRequestManager.performRequest(myRequest);
        while (myRequestManager.activeCount > 0) {
            myRequestManager.handleRequests();
            Async.onFrame();
        }

        // now we do multiple concurrent requests
        // set up 100 requests
        for (var i = 1; i < 50; ++i) {
            (function() {
                var p = i;
                myRequest = new Request("http://localhost:4042/"+i);
                myRequest.onDone = function () {
                    obj.testResponse = this;
                    ENSURE("obj.testResponse.responseString == '/"+p+"'");
                };
                // myRequest.verbose = true;
            })();
            myRequest.get();
            myRequestManager.performRequest(myRequest);
        }

        // handle them all
        while (myRequestManager.activeCount > 0) {
            msleep(100);
            myRequestManager.handleRequests();
            Async.onFrame();
        }
        
        obj.myServer = null;
    };

};

var myTestName = "testHttpServer.tst.js";
var mySuite = new UnitTestSuite(myTestName);

mySuite.addTest(new HttpServerUnitTest());
mySuite.run();
gc();
print(">> Finished test suite '" + myTestName + "', return status = " + mySuite.returnStatus() + "");
exit(mySuite.returnStatus());
