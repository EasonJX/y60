/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2001, ART+COM Berlin GmbH
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM GmbH Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM GmbH Berlin.
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
//    $RCSfile: UnitTest.js,v $
//
//   $Revision: 1.9 $
//
// Description: Utility Classes and Macros for easy unit testing
//
//
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/


use("Exception.js");
use("Y60JSSL.js");

function tail(thePath) {
    print(thePath);
    var myPath = new String(thePath);
    var myMatch = myPath.lastIndexOf("/");
    if (myMatch >= 0) {
        return myPath.substr(myMatch+1);
    }
    return thePath;
}

function UnitTest(theName) {
    this.Constructor(this, theName);
};


UnitTest.prototype.Constructor = function(obj, theName) {

    // private members:
    var _myName = theName;
    var _passedCount = 0;
    var _failedCount = 0;
    var _silentSuccess = false;
    var _abortOnFailure = false;

    // privileged member function
    obj.getFailedCount = function() {
        return _failedCount;
    };
    obj.getPassedCount = function() {
        return _passedCount;
    };
    obj.getMyName = function() {
        return _myName;
    };
    obj.setMyName = function(theName) {
        _myName = theName;
    };
    obj.incrementFailedCount = function() {
        ++_failedCount;
    };
    obj.incrementPassedCount = function() {
        ++_passedCount;
    };
    obj.incrementFailedCount = function() {
        ++_failedCount;
    };
    obj.incrementPassedCount = function() {
        ++_passedCount;
    };
    obj.setSilentSuccess = function(makeSilent) {
        if (makeSilent == undefined) {
            _silentSuccess = true;
        } else {
            _silentSuccess = makeSilent;
        }
    };
    obj.setAbortOnFailure = function(makeAbort) {
        if (makeAbort == undefined) {
            _abortOnFailure = true;
        } else {
            _abortOnFailure = makeAbort;
        }
    };
    obj.returnStatus = function() {
        if (_passedCount != 0 && _failedCount == 0) {
            return 0;
        } else {
            return -1;
        }
    };
    obj.ensure=function(/*bool*/ myExpressionResult,
            /*string*/ myExpression,
            /*string*/ mySourceLine)
    {
        var myPrefix = ">>>>>>  OK  ";
        if (myExpressionResult == true) {
            obj.incrementPassedCount();
            if (_silentSuccess) {
                return;
            }
        } else {
            obj.incrementFailedCount();
            myPrefix = "###### FAIL ";
        }
        print(myPrefix + " ("+ myExpression + "), Line " + mySourceLine + "");
        if (!myExpressionResult && _abortOnFailure) {
            print("UnitTest::ensure: Execution aborted");
            abort();
        }
    };
    obj.run = function() {};

    obj.getTracePrefix = function() {
        return ">>>>>> ";
    };
    obj.setup = function() {
        print(">>>> Launching Test Unit '" + _myName + "'");
    };
    obj.teardown = function() {
        print(">>>> Completed Test Unit '" + _myName + "'"
                + ", " + obj.getFailedCount() + " tests failed"
                + ", " + obj.getPassedCount() + " tests passed");
    };
    obj.setFailedCount = function(failedTests) {
        _failedCount = failedTests;
    };
    obj.setPassedCount = function(passedTests) {
        _passedCount = passedTests;
    };
    obj.ensureWith = function(theWithObj) {
        ourWithObj = theWithObj;
    };
};

function UnitTestSuite(theName) {
    this.Constructor(this, theName);
};

UnitTestSuite.prototype.Constructor = function(obj, theName) {

    UnitTest.prototype.Constructor(obj, theName);

    var _myTests = new Array();

    obj.addTest = function(myTest) {
        if (myTest) {
            _myTests.push(myTest);
            print(">> Added Test '" + myTest.getMyName() + "'");
        } else {
            print("## Failed to add Test");
            obj.incrementFailedCount();
        }
    }

    obj.run = function() {
        //print("UnitTestSuite.run()");
        try {
            try {
                obj.setup();
            } catch (e if e == Exception) {
                print("## An exception occured during setup of test suite '"
                        + obj.getMyName() << "':" + e + "");
                obj.incrementFailedCount();
                throw(e);
            } catch (e) {
                print("## An exception occured during setup of test suite '"
                        + obj.getMyName() << "':" + e + "");
                obj.incrementFailedCount();
                throw(e);
            }
            try {
                for (var i = 0; i < _myTests.length; ++i) {
                    _myTests[i].setup();
                    ourCurrentTest = _myTests[i];
                    _myTests[i].run();
                    obj.setFailedCount(obj.getFailedCount() + _myTests[i].getFailedCount());
                    obj.setPassedCount(obj.getPassedCount() + _myTests[i].getPassedCount());
                    _myTests[i].teardown();
                }
            } catch (e if e == Exception) {
                print("## An exception occured during execution of test suite '"
                        + obj.getMyName() << "':" + e + "");
                obj.incrementFailedCount();
                throw(e);
            } catch (e) {
                print("## An exception occured during execution of test suite '"
                        + obj.getMyName() << "':" + e + "");
                obj.incrementFailedCount();
                throw(e);
            }
            try {
                obj.teardown();
            } catch (e if e == Exception) {
                print("## An exception occured during teardown of test suite '"
                        + obj.getMyName() << "':" + e + "");
                obj.incrementFailedCount();
                throw(e);
            } catch (e) {
                print("## An exception occured during teardown of test suite '"
                        + obj.getMyName() << "':" + e + "");
                obj.incrementFailedCount();
                throw(e);
            }
        } catch (e) {
            print("## test suite '" + obj.getMyName() + "'"
                    + " was not completed because of an exception: "+e+"");
        }
    }
    obj.setup = function() {
        print(">> Launching Test Suite '" + obj.getMyName() + "'");
    }
    obj.teardown = function() {
        print(">> Completed Test Suite '" + obj.getMyName() + "'"
            + ", " + obj.getFailedCount() + " total tests failed"
            + ", " + obj.getPassedCount() + " total tests passed");
        obj.destroyMyTests();
    }
    obj.getNumTests = function() {
        return _myTests.length;
    }
    obj.getTest = function(theTestIndex) {
        return _myTests[theTestIndex];
    }
    obj.destroyMyTests = function() {
        while (_myTests.length) {
            _myTests.pop();
        }
    }
};

function DTITLE(TITLE) {
    print(ourCurrentTest.getTracePrefix() + TITLE + ":");
};

var ourCurrentTest = null;
var ourWithObj = [];

function DPRINT(theVariable) {
    var myExpression = "with (ourWithObj) {var obj = ourCurrentTest; with (obj) { "+theVariable+ "} } ";
    //var myExpression = "var obj = ourCurrentTest;"+theVariable;
    var myResult = eval(myExpression);
    print(ourCurrentTest.getTracePrefix() + "      "+ theVariable + " = " + myResult);
};

function DPRINT2(theMessage, theVariable) {
    var myExpression = "var obj = ourCurrentTest;"+theVariable;
    var myResult = eval(myExpression);
    print(ourCurrentTest.getTracePrefix() +"      "+ theMessage + ": " + theVariable + " = " + myResult);
};

function ENSURE(theExpression, theMessage) {
    var myExpression = "with (ourWithObj) {var obj = ourCurrentTest; with (obj) { "+theExpression+ "} } ";
    try {
        var myResult = eval(myExpression);
    } catch(e) {
        var myResult = false;
        print('###### EXCEPTION:'+e);
    }
    //dumpstack();
    //ourCurrentTest.ensure(myResult,theExpression,__LINE__(2),__FILE__(2));
    if (theMessage) {
        theExpression = theMessage;
    }
    ourCurrentTest.ensure(myResult,theExpression,__LINE__(2),theMessage);
    return myResult;
};

function FAILURE(MSG) {
    ourCurrentTest.ensure(false, MSG, __LINE__(2));
};
function SUCCESS(MSG) {
    ourCurrentTest.ensure(true, MSG, __LINE__(2));
};

function FAILURE2(MSG) {
    ourCurrentTest.ensure(false, MSG, __LINE__(3));
};
function SUCCESS2(MSG) {
    ourCurrentTest.ensure(true, MSG, __LINE__(3));
};


function ENSURE_EXCEPTION(theTest, theException) {
    try {
        var myExpression = "with (ourWithObj) {var obj = ourCurrentTest; with (obj) { "+theTest+ "} } ";
        eval(myExpression);
        FAILURE2(theTest);
    } catch (e) {
        if (e == theException || theException == "*" || theException == undefined) {
            SUCCESS2("EXCEPTION("+theTest+")");
        } else {
            FAILURE2("EXCEPTION("+theTest+")");
        }
    }
};

