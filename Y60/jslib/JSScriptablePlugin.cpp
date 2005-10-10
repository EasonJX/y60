//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: JSScriptablePlugin.cpp,v $
//   $Author: pavel $
//   $Revision: 1.7 $
//   $Date: 2005/04/24 00:41:19 $
//
//
//=============================================================================

#include "JSScriptablePlugin.h"
#include "IFactoryPlugin.h"
#include "JSNode.h"

#include <asl/PlugInManager.h>
#include <asl/Ptr.h>

#include <iostream>

using namespace std;
using namespace asl;
using namespace y60;
using namespace dom;

#define DB(x) // x

namespace jslib {

    static JSBool
    toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
        DOC_BEGIN("Prints 'JSScriptablePlugin'");
        DOC_END;
        std::string myStringRep = "JSScriptablePlugin";
        JSString * myString = JS_NewStringCopyN(cx,myStringRep.c_str(),myStringRep.size());
        *rval = STRING_TO_JSVAL(myString);
        return JS_TRUE;
    }

    JSScriptablePlugin::~JSScriptablePlugin() {
    }

    IScriptablePluginPtr &
    JSScriptablePlugin::getNative(JSContext *cx, JSObject *obj) {
        JSClass * myClass = JS_GetClass(obj);
        if (myClass->flags & JSCLASS_PRIVATE_IS_NSISUPPORTS) {
            JSScriptablePlugin * myJSScriptablePlugin = static_cast<JSScriptablePlugin*>(JS_GetPrivate(cx,obj));
            if (!myJSScriptablePlugin) {
                JS_ReportError(cx,"JSScriptablePlugin::getNative: internal error, binding object does not exist");
            }
            return myJSScriptablePlugin->_myNative;
        } else {
            throw Exception(std::string("Object is not a ScriptablePlugin: ") + myClass->name, PLUS_FILE_LINE);
        }
    }

    static JSBool
    onUpdateSettings(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
        DOC_BEGIN("Define a callback to be called when a given settings node changes.");
        DOC_PARAM("theConfigNode", DOC_TYPE_NODE);
        DOC_END;
        try {
            if (argc != 1) {
                JS_ReportError(cx, "JSScriptablePlugin::onUpdateSettings(): Wrong number of arguments. One (ConfigNode) expected");
                return JS_FALSE;
            }

            dom::NodePtr myConfigNode;
            if (JSVAL_IS_VOID(argv[0]) || !convertFrom(cx, argv[0], myConfigNode)) {
                JS_ReportError(cx, "JSScriptablePlugin::onUpdateSettings(): Argument #1 must be a node");
                return JS_FALSE;
            }

            JSScriptablePlugin::getNative(cx, obj)->onUpdateSettings(myConfigNode);
            return JS_TRUE;
        } HANDLE_CPP_EXCEPTION;
    }

    JSFunctionSpec *
    JSScriptablePlugin::Functions() {
        static JSFunctionSpec myFunctions[] = {
            // name                  native                   nargs
            {"toString",             toString,                0},
            {"onUpdateSettings",     onUpdateSettings,         0},
            {0}
        };
        return myFunctions;
    }

    JSPropertySpec *
    JSScriptablePlugin::Properties() {
        static JSPropertySpec myProperties[] = {
            {0}
        };
        return myProperties;
    }
    
    JSConstIntPropertySpec *
    JSScriptablePlugin::ConstIntProperties() {
        static JSConstIntPropertySpec myProperties[] = {{0}};
        return myProperties;
    }

    JSPropertySpec *
    JSScriptablePlugin::StaticProperties() {
        static JSPropertySpec myProperties[] = {{0}};
        return myProperties;
    }

    JSFunctionSpec *
    JSScriptablePlugin::StaticFunctions() {
        static JSFunctionSpec myFunctions[] = {{0}};
        return myFunctions;
    }

    static JSBool
    getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
        try {
            if (JSVAL_IS_INT(id)) {
                int myIndex = JSVAL_TO_INT(id);
                AC_WARNING << "Unsupported: getProperty called with index=" << myIndex;
            } else {
                JSString * myJSStr = JS_ValueToString(cx, id);
                std::string myProperty = JS_GetStringBytes(myJSStr);
                PropertyValue myPropertyValue(cx, vp);
                JSScriptablePlugin::getNative(cx, obj)->onGetProperty(myProperty, myPropertyValue);
            }
        } HANDLE_CPP_EXCEPTION;
        return JS_TRUE;
    }

    static JSBool
    setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
        try {
            if (JSVAL_IS_INT(id)) {
                int myIndex = JSVAL_TO_INT(id);
                AC_WARNING << "Unsupported: setProperty called with index=" << myIndex;
            } else {
                JSString * myJSStr = JS_ValueToString(cx, id);
                std::string myProperty = JS_GetStringBytes(myJSStr);
                PropertyValue myPropertyValue(cx, vp);
                JSScriptablePlugin::getNative(cx, obj)->onSetProperty(myProperty, myPropertyValue);
            }
        } HANDLE_CPP_EXCEPTION;
        return JS_TRUE;
    }

    JSBool
    JSScriptablePlugin::Constructor(JSContext *cx, JSObject * obj, uintN argc, jsval *argv, jsval *rval) {
        AC_TRACE << "JSScriptablePlugin::Constructor: " << JSA_GetClass(cx,obj)->name;
        DOC_BEGIN("Constructs a ScriptablePlugin.");
        DOC_END;
        try {
            const char * myClassName = JSA_GetClass(cx,obj)->name;
            if (JSA_GetClass(cx,obj) != Class(myClassName)) {
                JS_ReportError(cx,"Constructor for %s bad object; did you forget a 'new'?", myClassName);
                return JS_FALSE;
            }

            if (argc > 1) {
                JS_ReportError(cx, "Constructor for %s: bad number of arguments: expected 0 or 1 "
                    "(BehaviourNode), got %d", myClassName, argc);
                return JS_FALSE;
            }

            PlugInBasePtr myPlugIn;
            try {
                myPlugIn = PlugInManager::get().getPlugIn(myClassName);
            } catch(PlugInException & theException)  {
                JS_ReportError(cx, "JSScriptablePlugin::Constructor: %s", as_string(theException).c_str());
                return JS_FALSE;
            }

            IScriptablePluginPtr myScriptablePlugin;
            IFactoryPluginPtr myFactoryPlugin = dynamic_cast_Ptr<IFactoryPlugin>(myPlugIn);
            if (myFactoryPlugin) {
                myScriptablePlugin = myFactoryPlugin->createInstance();
            } else {
                myScriptablePlugin = dynamic_cast_Ptr<IScriptablePlugin>(myPlugIn);
                if (!myScriptablePlugin) {
                    JS_ReportError(cx, "JSScriptablePlugin::Constructor: plug-in does not implement IScriptablePlugin");
                    return JS_FALSE;
                }
            }

            JSScriptablePlugin * myNewObject = new JSScriptablePlugin(myScriptablePlugin);
            JS_SetPrivate(cx, obj, myNewObject);

            return JS_TRUE;
        } HANDLE_CPP_EXCEPTION;
    }

    static void finalizeImpl(JSContext *cx, JSObject *obj) {
        JSScriptablePlugin * myImpl = static_cast<JSScriptablePlugin*>(JS_GetPrivate(cx,obj));
        delete myImpl;
    }

    JSClass *
    JSScriptablePlugin::Class(const char * theClassName) {
        typedef std::map<const char *, JSClass> ClassMap;
        static ClassMap _ourClassMap;
        ClassMap::iterator myIt = _ourClassMap.find(theClassName);
        if (myIt != _ourClassMap.end()) {
            return &myIt->second;
        } else {
            JSClass myTempClass = {
                theClassName,                         // const char          *name;
                JSCLASS_HAS_PRIVATE |                 // uint32              flags;
                //JSCLASS_HAS_PRIVATE |               /* objects have private slot */
                //JSCLASS_NEW_ENUMERATE |             /* has JSNewEnumerateOp hook */
                //JSCLASS_NEW_RESOLVE //|                 /* has JSNewResolveOp hook */
                JSCLASS_PRIVATE_IS_NSISUPPORTS |    /* private is (nsISupports *) */
                //JSCLASS_SHARE_ALL_PROPERTIES |      /* all properties are SHARED */
                //JSCLASS_NEW_RESOLVE_GETS_START      //JSNewResolveOp gets starting
                                                      //object in prototype chain
                                                      //passed in via *objp in/out
                                                      //parameter */
                0,
                /* Mandatory non-null function pointer members. */
                NoisyAddProperty,    // JSPropertyOp        addProperty;
                NoisyDelProperty,    // JSPropertyOp        delProperty;
                getProperty,         // JSPropertyOp        getProperty;
                setProperty,         // JSPropertyOp        setProperty;
                NoisyEnumerate,      // JSEnumerateOp       enumerate;
                NoisyResolve,        // JSResolveOp         resolve;
                //(JSResolveOp)newResolve,  // JSResolveOp         resolve;
                NoisyConvert,        // JSConvertOp         convert;
                finalizeImpl,           // JSFinalizeOp        finalize;

                JSCLASS_NO_OPTIONAL_MEMBERS
                /* Optionally non-null members start here. */
                                    // JSGetObjectOps      getObjectOps;
                                    // JSCheckAccessOp     checkAccess;
                                    // JSNative            call;
                                    // JSNative            construct;
                                    // JSXDRObjectOp       xdrObject;
                                    // JSHasInstanceOp     hasInstance;
                                    // JSMarkOp            mark;
                                    // jsword              spare;
            };
            _ourClassMap[theClassName] = myTempClass;
            return &_ourClassMap[theClassName];
        }
    }


    JSObject *
    JSScriptablePlugin::Construct(JSContext *cx, IScriptablePluginPtr theNative) {
        JSObject * myNewObj = JS_ConstructObject(cx, Class(theNative->ClassName()), 0, 0);
        JSScriptablePlugin * myWrapper = static_cast<JSScriptablePlugin*>(JS_GetPrivate(cx,myNewObj));
        myWrapper->_myNative = theNative;
        return myNewObj;
    }

    void
    JSScriptablePlugin::initClass(JSContext *cx, JSObject * theGlobalObject, const char * theClassName) {
        DB(AC_TRACE << "JSScriptablePlugin::initClass for class " << theClassName << endl;)

        JSObject * myClassObject = JS_InitClass(cx, theGlobalObject, NULL, Class(theClassName),
                Constructor, 0, Properties(), Functions(), 0, 0);
         
        //document the plugin mechanism and not the plugin named theClassName itself...
        createClassModuleDocumentation("global", "JSScriptablePlugin", Properties(),
                                       Functions(), ConstIntProperties(), 
                                       StaticProperties(), StaticFunctions(), ""); 
        documentConstructor("global", "JSScriptablePlugin", Constructor);

        jsval myConstructorFuncObjVal;
        if (JS_GetProperty(cx, theGlobalObject, theClassName, &myConstructorFuncObjVal)) {
        } else {
            AC_ERROR << "JSScriptablePlugin::initClass(): constructor function object not found, " <<
                "could not initialize static members"<<endl;
        }
    }

    jsval as_jsval(JSContext *cx, IScriptablePluginPtr theNative) {
        JSObject * myReturnObject = JSScriptablePlugin::Construct(cx, theNative);
        return OBJECT_TO_JSVAL(myReturnObject);
    }

    bool convertFrom(JSContext * cx, jsval theValue, IScriptablePluginPtr & thePtr) {
        if (JSVAL_IS_OBJECT(theValue)) {
            JSObject * obj;
            JSObject * obj2 = JSVAL_TO_OBJECT(theValue);
            if (JS_ValueToObject(cx, theValue, &obj)) {
                thePtr = JSScriptablePlugin::getNative(cx, obj);
                return true;
            }
        }
        return false;
    }
}
