//=============================================================================
// Copyright (C) 2003-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include <asl/settings.h>

#include <iostream>

#include <y60/jssettings.h>

#include "SDLApp.h"
#include "SDLFontInfo.h"
#include <SDL/SDL.h>
#include <SDL/SDL_syswm.h>

#include "JSRenderWindow.h"

#include <y60/GLAlloc.h>
#include <y60/RenderStatistic.h>
#include <y60/Renderer.h>

#include <y60/Event.h>

#include <asl/Time.h>
#include <asl/numeric_functions.h>
#include <asl/GeometryUtils.h>
#include <asl/Box.h>
#include <asl/PlugInManager.h>

#include <y60/JSBox.h>
#include <y60/JSLine.h>
#include <y60/JSTriangle.h>
#include <y60/JSSphere.h>
#include <y60/JSVector.h>
#include <y60/JSMatrix.h>
#include <y60/JSNode.h>
#include <y60/JSNodeList.h>
#include <y60/JSAbstractRenderWindow.h>

#define DB(x) // x

using namespace std;
using namespace y60;
using namespace jslib;

// typedef std::map<string, KeyEvent> KeyMap;
typedef SDLWindow NATIVE;
typedef jslib::AbstractRenderWindow BASE;
typedef jslib::JSAbstractRenderWindow<SDLWindow> JSBASE;

namespace jslib {

template <>
struct JSClassTraits<AbstractRenderWindow> : public JSClassTraitsWrapper<SDLWindow, JSRenderWindow> {
    static JSClass * Class() {
        return JSRenderWindow::Base::Class();
    }
};

}

// =============== Wrapper Implementation

// =============== Own Methods
static JSBool
resize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Resizes the window. Optionally the video mode is set to fullscreen and the screen resolution is set so it fits the current window size best.");
    DOC_PARAM("theWidth", "Target window width in pixel", DOC_TYPE_INTEGER);
    DOC_PARAM("theHeight", "Target window height in pixel", DOC_TYPE_INTEGER);
    DOC_PARAM_OPT("theFullscreenFlag", "If this flag is set to true, the video mode is set to fullscreen.", DOC_TYPE_INTEGER, false);
    DOC_END;
    ensureParamCount(argc, 2, 3);
    if (argc == 2) {
        unsigned myWidth;
        unsigned myHeight;
        if (!convertFrom(cx, argv[0], myWidth) || !convertFrom(cx, argv[1], myHeight)) {
            JS_ReportError(cx, "Renderer::resize(): Argument one and two must be integers");
            return JS_FALSE;
        }
        JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
        myObj.getNative().setVideoMode(myWidth, myHeight);
        return JS_TRUE;
    } else { // argc == 3
        return Method<NATIVE>::call(&NATIVE::setVideoMode,cx,obj,argc,argv,rval);
    }
}
static JSBool
createCursor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    return Method<SDLWindow>::call(&SDLWindow::createCursor,cx,obj,argc,argv,rval);
}
static JSBool
resetCursor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    return Method<SDLWindow>::call(&SDLWindow::resetCursor,cx,obj,argc,argv,rval);
}
static JSBool
stop(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Opens the window, if it is not alread open and starts the main renderloop.");
    DOC_END;
    return Method<SDLWindow>::call(&SDLWindow::stop,cx,obj,argc,argv,rval);
}
static JSBool
go(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Closes the window and stops the main renderloop");
    DOC_END;
    return Method<SDLWindow>::call(&SDLWindow::go,cx,obj,argc,argv,rval);
}
static JSBool
loadTTF(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Load a TTF font");
    DOC_PARAM("theName", "Symbolic font name", DOC_TYPE_STRING);
    DOC_PARAM("theFilename", "Filename to load the font from", DOC_TYPE_STRING);
    DOC_PARAM("theHeight", "Font height", DOC_TYPE_INTEGER);
    DOC_PARAM_OPT("theFontFace", "Font face", DOC_TYPE_INTEGER, SDLFontInfo::NORMAL);
    DOC_END;
    // Binding is implemented by hand to allow overloading
    try {
        SDLFontInfo::FONTTYPE myFontType = SDLFontInfo::NORMAL;
        std::string myName   = "";
        std::string myPath   = "";
        unsigned    myHeight = 0;

        if (argc != 3 && argc != 4) {
            JS_ReportError(cx, "Renderer::loadTTF(): Wrong number of arguments. Must be three or four");
            return JS_FALSE;
        }

        JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);

        if (!convertFrom(cx, argv[0], myName)) {
            JS_ReportError(cx, "Renderer::loadTTF(): Argument #1 must be a font name");
            return JS_FALSE;
        }
        if (!convertFrom(cx, argv[1], myPath)) {
            JS_ReportError(cx, "Renderer::loadTTF(): Argument #2 must be a font path");
            return JS_FALSE;
        }
        if (!convertFrom(cx, argv[2], myHeight)) {
            JS_ReportError(cx, "Renderer::loadTTF(): Argument #3 must be a font height");
            return JS_FALSE;
        }

        if (argc > 3) {
            unsigned short myFontTypeEnum = 0;
            if (!convertFrom(cx, argv[3], myFontTypeEnum)) {
                JS_ReportError(cx, "Renderer::loadTTF(): Argument #4 must be a font face type");
                return JS_FALSE;
            }
            myFontType = SDLFontInfo::FONTTYPE(myFontTypeEnum);
        }

        myObj.getNative().getRenderer()->getTextManager().loadTTF(myName, myPath, myHeight, myFontType);
        return JS_TRUE;
   } HANDLE_CPP_EXCEPTION;
}

static JSBool
setMousePosition(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Set the mouse cursor position.");
    DOC_PARAM("theX", "X-Position in pixels", DOC_TYPE_INTEGER);
    DOC_PARAM("theY", "Y-Position in pixels", DOC_TYPE_INTEGER);
    DOC_END;
    return Method<SDLWindow>::call(&SDLWindow::setMousePosition,cx,obj,argc,argv,rval);
}

static JSBool
draw(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    if (argc == 3) {
        if (JSLineSegment::matchesClassOf(cx, argv[0])) {
            typedef void (SDLWindow::*MyMethod)(
                const asl::LineSegment<float> &,
                const asl::Vector4f & theColor,
                const asl::Matrix4f & theTransformation);
            return Method<SDLWindow>::call((MyMethod)&SDLWindow::draw,cx,obj,argc,argv,rval);
        }
        if (JSTriangle::matchesClassOf(cx, argv[0])) {
            typedef void (SDLWindow::*MyMethod)(
                const asl::Triangle<float> &,
                const asl::Vector4f & theColor,
                const asl::Matrix4f & theTransformation);
            return Method<SDLWindow>::call((MyMethod)&SDLWindow::draw,cx,obj,argc,argv,rval);
        }
        if (JSSphere::matchesClassOf(cx, argv[0])) {
            typedef void (SDLWindow::*MyMethod)(
                const asl::Sphere<float> &,
                const asl::Vector4f & theColor,
                const asl::Matrix4f & theTransformation);
            return Method<SDLWindow>::call((MyMethod)&SDLWindow::draw,cx,obj,argc,argv,rval);
        }
        if (JSBox3f::matchesClassOf(cx, argv[0])) {
            typedef void (SDLWindow::*MyMethod)(
                const asl::Box3<float> &,
                const asl::Vector4f & theColor,
                const asl::Matrix4f & theTransformation);
            return Method<SDLWindow>::call((MyMethod)&SDLWindow::draw,cx,obj,argc,argv,rval);
        }
        JS_ReportError(cx,"JSRenderWindow::draw: bad argument type #0");
        return JS_FALSE;
    }
    JS_ReportError(cx,"JSRenderWindow::draw: bad number of arguments, 3 expected");
    return JS_FALSE;
}

static JSBool
setEventRecorderMode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Set the EventRecorder mode");
    DOC_PARAM("theMode", "STOP, PLAY, or RECORD", DOC_TYPE_INTEGER);
    DOC_PARAM_OPT("theDiscardFlag", "When theMODE==RECORD, discard previously recorded events", DOC_TYPE_INTEGER, true);
    DOC_END;
    if (argc >= 1) {
        unsigned myMode;
        bool myDiscardFlag = true;
        if (!convertFrom(cx,argv[0],myMode)) {
            JS_ReportError(cx,"JSRenderWindow::setEventRecorderMode: parameter 0 must be an int");
            return JS_FALSE;
        }

        if (argc > 1 && !convertFrom(cx,argv[1],myDiscardFlag)) {
            JS_ReportError(cx,"jsrenderwindow::setEventRecorderMode: parameter 1 must be an bool");
            return JS_FALSE;
        }

        JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
        myObj.getNative().setEventRecorderMode((EventRecorder::Mode) myMode, myDiscardFlag);

        return JS_TRUE;
    }
    JS_ReportError(cx,"JSRenderWindow::setEventRecorderMode: bad number of arguments, 1 expected");
    return JS_FALSE;
}

static JSBool
getEventRecorderMode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Returns current mode of the EventRecorder");
    DOC_RVAL("theMode", DOC_TYPE_INTEGER);
    DOC_END;
    return Method<SDLWindow>::call(&SDLWindow::getEventRecorderMode,cx,obj,argc,argv,rval);
}

static JSBool
loadEvents(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Load events from file");
    DOC_PARAM("theFilename", "Filename to load from", DOC_TYPE_STRING);
    DOC_END;
    return Method<SDLWindow>::call(&SDLWindow::loadEvents,cx,obj,argc,argv,rval);
}

static JSBool
saveEvents(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Save previously recorded events to file");
    DOC_PARAM("theFilename", "Filename to save to", DOC_TYPE_STRING);
    DOC_END;
    return Method<SDLWindow>::call(&SDLWindow::saveEvents,cx,obj,argc,argv,rval);
}

JSFunctionSpec *
JSRenderWindow::Functions() {
    static JSFunctionSpec myFunctions[] = {
        /* name                DERIVED::native          nargs    */
        {"resize",             resize,                   3},
        {"resetCursor",        resetCursor,              0},
        {"createCursor",       createCursor,             1},
        {"go",                 go,                       0},
        {"stop",               stop,                     0},
        {"loadTTF",            loadTTF,                  4},
        {"setMousePosition",   setMousePosition,         2},
        {"draw",               draw,                     1},
        {"setEventRecorderMode", setEventRecorderMode,   1},
        {"getEventRecorderMode", getEventRecorderMode,   0},
        {"loadEvents",           loadEvents,             1},
        {"saveEvents",           saveEvents,             1},
        {0}
    };
    return myFunctions;
}

enum PropertyNumbers {
    PROP_windeco = JSBASE::PROP_END,
    PROP_showMouseCursor,
    PROP_showTaskbar,
    PROP_captureMouseCursor,
    PROP_autoPause,
    PROP_title,
    PROP_position,
    PROP_screenSize,
    // ConstInt
    PROP_STOP,
    PROP_PLAY,
    PROP_RECORD
};

JSConstIntPropertySpec *
JSRenderWindow::ConstIntProperties() {

#if 0
    const unsigned short PROP_BOLD       = 1;
    const unsigned short PROP_ITALIC     = 2;
    const unsigned short PROP_BOLDITALIC = 3;
#endif

    static JSConstIntPropertySpec myProperties[] = {
        { "STOP", PROP_STOP, y60::EventRecorder::STOP },
        { "PLAY", PROP_PLAY, y60::EventRecorder::PLAY },
        { "RECORD", PROP_RECORD, y60::EventRecorder::RECORD },
        {0}
    };
    return myProperties;
};

JSPropertySpec *
JSRenderWindow::Properties() {
    static JSPropertySpec myProperties[] = {
        {"decorations",        PROP_windeco,            JSPROP_ENUMERATE|JSPROP_PERMANENT}, // boolean
        {"showMouseCursor",    PROP_showMouseCursor,    JSPROP_ENUMERATE|JSPROP_PERMANENT}, // boolean
        {"showTaskbar",        PROP_showTaskbar,        JSPROP_ENUMERATE|JSPROP_PERMANENT}, // boolean
        {"captureMouseCursor", PROP_captureMouseCursor, JSPROP_ENUMERATE|JSPROP_PERMANENT}, // boolean
        {"autoPause",          PROP_autoPause,          JSPROP_ENUMERATE|JSPROP_PERMANENT}, // boolean
        {"title",              PROP_title,              JSPROP_ENUMERATE|JSPROP_PERMANENT}, // boolean
        {"screenSize",         PROP_screenSize,         JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"position",           PROP_position,           JSPROP_ENUMERATE|JSPROP_PERMANENT}, // Vector2i
        {0}
    };
    return myProperties;
}

// getproperty handling
JSBool
JSRenderWindow::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);

    switch (theID) {
        case PROP_windeco:
            *vp = as_jsval(cx, myObj.getNative().getWinDeco());
            return JS_TRUE;
        case PROP_showMouseCursor:
            *vp = as_jsval(cx, myObj.getNative().getShowMouseCursor());
            return JS_TRUE;
        case PROP_showTaskbar:
            *vp = as_jsval(cx, myObj.getNative().getShowTaskbar());
            return JS_TRUE;
        case PROP_captureMouseCursor:
            *vp = as_jsval(cx, myObj.getNative().getCaptureMouseCursor());
            return JS_TRUE;
        case PROP_autoPause:
            *vp = as_jsval(cx, myObj.getNative().getAutoPause());
            return JS_TRUE;
        case PROP_title:
            *vp = as_jsval(cx, myObj.getNative().getWindowTitle());
            return JS_TRUE;
        case PROP_position:
            *vp = as_jsval(cx, myObj.getNative().getPosition());
            return JS_TRUE;
        case PROP_screenSize:
            *vp = as_jsval(cx, myObj.getNative().getScreenSize());
            return JS_TRUE;
        default:
            return JSBASE::getPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
    }
}

JSBool JSRenderWindow::getPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    return JS_TRUE;
}

// setproperty handling
JSBool
JSRenderWindow::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    jsval dummy;
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);

    switch (theID) {
        case PROP_windeco:
            return Method<SDLWindow>::call(&SDLWindow::setWinDeco, cx, obj, 1, vp, &dummy);
        case PROP_showMouseCursor:
            return Method<SDLWindow>::call(&SDLWindow::setShowMouseCursor, cx, obj, 1, vp, &dummy);
        case PROP_showTaskbar:
            return Method<SDLWindow>::call(&SDLWindow::setShowTaskbar, cx, obj, 1, vp, &dummy);
        case PROP_captureMouseCursor:
            return Method<SDLWindow>::call(&SDLWindow::setCaptureMouseCursor, cx, obj, 1, vp, &dummy);
        case PROP_autoPause:
            return Method<SDLWindow>::call(&SDLWindow::setAutoPause, cx, obj, 1, vp, &dummy);
        case PROP_title:
            return Method<SDLWindow>::call(&SDLWindow::setWindowTitle, cx, obj, 1, vp, &dummy);
        case PROP_position:
            return Method<SDLWindow>::call(&SDLWindow::setPosition, cx, obj, 1, vp, &dummy);
        default:
            return JSBASE::setPropertySwitch(myObj.getNative(),theID, cx, obj, id, vp);
    }
}

JSBool
JSRenderWindow::setPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    return JS_TRUE;
}

JSBool
JSRenderWindow::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Creates a new SDL window. Call go() to open it.");
    DOC_RVAL("theNewWindow", DOC_TYPE_OBJECT);
    DOC_END;
    IF_NOISY2(AC_TRACE << "Constructor argc =" << argc << endl);
    if (JSA_GetClass(cx,obj) != Base::Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }
    if (argc != 0) {
        JS_ReportError(cx,"JSRenderWindow::Constructor: bad number of arguments, must be 0");
        return JS_FALSE;
    }

    OWNERPTR myNewWindow = NATIVE::create();
    JSRenderWindow * myNewObject = new JSRenderWindow(myNewWindow, &(*myNewWindow));
    if (myNewObject) {
        JS_SetPrivate(cx, obj, myNewObject);

        // Set the JavaScript RenderWindow object as default event listener for the new window
        myNewWindow->setEventListener(obj);
        myNewWindow->setJSContext(cx);
        return JS_TRUE;
    }
    JS_ReportError(cx,"JSRenderWindow::Constructor: new JSRenderWindow failed");
    return JS_FALSE;
}

void
JSRenderWindow::addClassProperties(JSContext * cx, JSObject * theClassProto) {
    JSA_AddFunctions(cx, theClassProto, JSBASE::BaseFunctions());
    JSA_AddProperties(cx, theClassProto, JSBASE::BaseProperties());
    createClassDocumentation(ClassName(), JSBASE::BaseProperties(),
            JSBASE::BaseFunctions(), 0, 0, 0);
    DOC_CREATE(JSRenderWindow);

}

JSObject *
JSRenderWindow::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject * myClassObject = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, Properties(), Functions(), ConstIntProperties());
    if (myClassObject) {
        addClassProperties(cx, myClassObject);
    }
    jsval myConstructorFuncObjVal;
    if (JS_GetProperty(cx, theGlobalObject, ClassName(), &myConstructorFuncObjVal)) {
        JSObject * myConstructorFuncObj = JSVAL_TO_OBJECT(myConstructorFuncObjVal);
        // add static props & methods
        JSA_DefineConstInts(cx, myConstructorFuncObj, JSBASE::ConstIntProperties());
        JSA_AddFunctions(cx, myConstructorFuncObj, JSBASE::BaseStaticFunctions());
    } else {
        AC_ERROR << "JSRenderWindow::initClass: constructor function object not found, could not initialize static members"<<endl;
    }
    return myClassObject;
}

JSPropertySpec *
JSRenderWindow::StaticProperties() {
    static JSPropertySpec myProperties[] = {{0}};
    return myProperties;
}

JSFunctionSpec *
JSRenderWindow::StaticFunctions() {
    static JSFunctionSpec myFunctions[] = {{0}};
    return myFunctions;
}

namespace jslib {

bool convertFrom(JSContext *cx, jsval theValue, SDLWindow *& theRenderWindow) {
    if (JSVAL_IS_OBJECT(theValue)) {
        JSObject * myArgument;
        if (JS_ValueToObject(cx, theValue, &myArgument)) {
            if (JSA_GetClass(cx,myArgument) == JSClassTraits<JSRenderWindow::NATIVE>::Class()) {
                theRenderWindow = &(*JSClassTraits<JSRenderWindow::NATIVE>::getNativeOwner(cx,myArgument));
                return true;
            }
        }
    }
    return false;
}

jsval as_jsval(JSContext *cx, asl::Ptr<SDLWindow> theOwner) {
    JSObject * myReturnObject = JSRenderWindow::Construct(cx, theOwner, &(*theOwner));
    return OBJECT_TO_JSVAL(myReturnObject);
}

}
