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

#include "JSTask.h"
#include "JSTaskWindow.h"
#include <y60/jslib/JSRenderer.h>
#include <y60/jsbase/JScppUtils.h>
#include <y60/jsbase/JScppUtils.impl>
#include <y60/jsbase/JSWrapper.impl>

#include <iostream>

using namespace std;
using namespace asl;
using namespace y60;

#define DB(x) // x

namespace jslib {

    template class JSWrapper<y60::Task, asl::Ptr<y60::Task> , jslib::StaticAccessProtocol>;

    static JSBool
    terminate(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
        DOC_BEGIN("");
        DOC_END;
        return Method<JSTask::NATIVE>::call(&JSTask::NATIVE::terminate,cx,obj,argc,argv,rval);
    }
    static JSBool
    captureDesktop(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
        DOC_BEGIN("");
        DOC_END;
        return Method<JSTask::NATIVE>::call(&JSTask::NATIVE::captureDesktop,cx,obj,argc,argv,rval);
    }
    static JSBool
    waitForInputIdle(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
        DOC_BEGIN("");
        DOC_END;
        return Method<JSTask::NATIVE>::call(&JSTask::NATIVE::waitForInputIdle,cx,obj,argc,argv,rval);
    }
    static JSBool
    setPriority(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
        DOC_BEGIN("");
        DOC_END;
        return Method<JSTask::NATIVE>::call(&JSTask::NATIVE::setPriority,cx,obj,argc,argv,rval);
    }
    static JSBool
    addExternalWindow(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
        DOC_BEGIN("Adds windows to this task, that have been created by other processes");
        DOC_PARAM("theWindowName", "", DOC_TYPE_STRING);
        DOC_END;
        return Method<JSTask::NATIVE>::call(&JSTask::NATIVE::addExternalWindow,cx,obj,argc,argv,rval);
    }

    JSFunctionSpec *
    JSTask::Functions() {
        static JSFunctionSpec myFunctions[] = {
            // name                  native            nargs
            {"terminate",            terminate,         0},
            {"captureDesktop",       captureDesktop,    1},
            {"waitForInputIdle",     waitForInputIdle,  1},
            {"setPriority",          setPriority,       1},
            {"addExternalWindow",    addExternalWindow, 1},
            {0}
        };
        return myFunctions;
    }

    JSPropertySpec *
    JSTask::Properties() {
        static JSPropertySpec myProperties[] = {
            {"isActive", PROP_isActive, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT},
            {"windows",  PROP_windows, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT},
            {0}
        };
        return myProperties;
    }

    JSPropertySpec *
    JSTask::StaticProperties() {
        static JSPropertySpec myProperties[] = {
            {0}
        };
        return myProperties;
    }

    JSFunctionSpec *
    JSTask::StaticFunctions() {
        static JSFunctionSpec myFunctions[] = {
            {0}
        };
        return myFunctions;
    }

    // getproperty handling
    JSBool
    JSTask::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
        switch (theID) {
            case PROP_isActive:
                *vp = as_jsval(cx, getNative().isActive());
                return JS_TRUE;
            case PROP_windows:
                *vp = as_jsval(cx, getNative().getWindows());
                return JS_TRUE;
            default:
                JS_ReportError(cx,"JSTask::getProperty: index %d out of range", theID);
                return JS_FALSE;
        }
    }

    // setproperty handling
    JSBool
    JSTask::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
        switch (theID) {
            case PROP_isActive:
                //jsval dummy;
                //return Method<NATIVE>::call(&NATIVE::isOpen, cx, obj, 1, vp, &dummy);
                return JS_FALSE;
            default:
                JS_ReportError(cx,"JSTask::setPropertySwitch: index %d out of range", theID);
                return JS_FALSE;
        }
    }

    JSBool
    JSTask::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
        DOC_BEGIN("Creates a Task Object");
        DOC_PARAM("theCommand", "", DOC_TYPE_STRING);
        DOC_RESET;
        DOC_PARAM("theRenderer", "", DOC_TYPE_OBJECT);
        DOC_PARAM_OPT("thePath", "", DOC_TYPE_STRING, "");
        DOC_PARAM_OPT("theShowFlag", "", DOC_TYPE_BOOLEAN, "true");
        DOC_END;
        if (JSA_GetClass(cx,obj) != Class()) {
            JS_ReportError(cx,"Constructor for %s bad object; did you forget a 'new'?", ClassName());
            return JS_FALSE;
        }

        if (argc < 1) {
            JS_ReportError(cx,"JSTask::Constructor: bad argument count 0");
            return JS_FALSE;
        }

        Renderer* myRenderer = 0;
        string myCommand = "";

        if (JSVAL_IS_VOID(argv[0])) {
            JS_ReportError(cx,"JSTask::Constructor: bad argument #1, must be a string (command) or a renderer");
            return JS_FALSE;
        }
        if (!convertFrom(cx, argv[0], myRenderer)) {
            if (!convertFrom(cx, argv[0], myCommand)) {
                JS_ReportError(cx,"JSTask::Constructor: bad argument #1, must be a string (command) or a renderer");
                return JS_FALSE;
            }
        }

        string myPath = "";
        if (argc > 1 && !JSVAL_IS_VOID(argv[1]) && !convertFrom(cx, argv[1], myPath)) {
            JS_ReportError(cx,"JSTask::Constructor: bad argument #2, must be a string (path)");
            return JS_FALSE;
        }

        bool myShowFlag = true;
        if (argc > 2 && !JSVAL_IS_VOID(argv[2]) && !convertFrom(cx, argv[2], myShowFlag)) {
            JS_ReportError(cx,"JSTask::Constructor: bad argument #3, must be a bool (showflag)");
            return JS_FALSE;
        }
        JSTask * myNewObject;

        if (myCommand.size() > 0) {
            OWNERPTR myNewNative = OWNERPTR(new Task(myCommand, myPath, myShowFlag));
            myNewObject = new JSTask(myNewNative, &*myNewNative);
        } else {
            OWNERPTR myNewNative = OWNERPTR(new Task());
            myNewObject = new JSTask(myNewNative, &*myNewNative);
        }

        if (myNewObject) {
            JS_SetPrivate(cx, obj, myNewObject);
            return JS_TRUE;
        }
        JS_ReportError(cx,"JSTask::Constructor: bad parameters");
        return JS_FALSE;
    }

#define DEFINE_TASK_FLAG(NAME) { #NAME, PROP_ ## NAME , y60::Task::NAME }

    JSConstIntPropertySpec *
    JSTask::ConstIntProperties() {
        static JSConstIntPropertySpec myProperties[] = {
            DEFINE_TASK_FLAG(ABOVE_NORMAL),
            DEFINE_TASK_FLAG(BELOW_NORMAL),
            DEFINE_TASK_FLAG(HIGH_PRIORITY),
            DEFINE_TASK_FLAG(IDLE_PRIORITY),
            DEFINE_TASK_FLAG(NORMAL_PRIORITY),
            DEFINE_TASK_FLAG(REALTIME_PRIORITY),
            {0}
        };
        return myProperties;
    }

    JSObject *
    JSTask::initClass(JSContext *cx, JSObject *theGlobalObject) {
        JSObject *myClass = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, Properties(), Functions(), ConstIntProperties());
        DOC_MODULE_CREATE("Components", JSTask);
        return myClass;
    }

    using namespace y60;
    jsval as_jsval(JSContext *cx, JSTask::OWNERPTR theOwner) {
        JSObject * myReturnObject = JSTask::Construct(cx, theOwner, &(*theOwner));
        return OBJECT_TO_JSVAL(myReturnObject);
    }

    jsval as_jsval(JSContext *cx, JSTask::OWNERPTR theOwner, JSTask::NATIVE * theNative) {
        JSObject * myObject = JSTask::Construct(cx, theOwner, theNative);
        return OBJECT_TO_JSVAL(myObject);
    }

    template jsval as_jsval(JSContext *cx, const std::vector<JSTaskWindow::OWNERPTR> & theVector);
}
