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
//   $RCSfile: JSTNTMeasurementList.cpp,v $
//   $Author: martin $
//   $Revision: 1.9 $
//   $Date: 2005/04/21 16:25:03 $
//
//
//=============================================================================

#include "JSTNTMeasurementList.h"
#include "JSTreeIter.h"
#include "JSSignalProxies.h"
#include "jsgtk.h"
#include <y60/JSNode.h>
#include <y60/JScppUtils.h>
#include <iostream>

using namespace std;
using namespace asl;

namespace jslib {


static JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    std::string myStringRep = string("Gtk::TNTMeasurementList@") + as_string(obj);
    JSString * myString = JS_NewStringCopyN(cx,myStringRep.c_str(),myStringRep.size());
    *rval = STRING_TO_JSVAL(myString);
    return JS_TRUE;
}

static JSBool
registerTypeIcon(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    
    try {

        ensureParamCount(argc, 2);

        acgtk::TNTMeasurementList * myNative;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);

        std::string myTypeName;
        if ( ! convertFrom(cx, argv[0], myTypeName)) {
            JS_ReportError(cx, "TNTMeasurementList::registerTypeIcon(): Argument 0 must be a string.");
            return JS_FALSE;
        }

        std::string myIconFile;
        if ( ! convertFrom(cx, argv[1], myIconFile)) {
            JS_ReportError(cx, "TNTMeasurementList::registerTypeIcon(): Argument 1 must be a string.");
            return JS_FALSE;
        }

        std::string myIconWithPath = searchFileRelativeToJSInclude(cx, obj, argc, argv, myIconFile);

        myNative->registerTypeIcon(myTypeName, myIconWithPath);
        return JS_TRUE;

    } HANDLE_CPP_EXCEPTION;
    
}

static JSBool
append(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    
    try {

        ensureParamCount(argc, 2);

        acgtk::TNTMeasurementList * myNative;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);

        dom::NodePtr myMeasurementNode;
        if ( ! convertFrom(cx, argv[0], myMeasurementNode)) {
            JS_ReportError(cx, "TNTMeasurementList::append(): Argument 0 must be a xml node.");
            return JS_FALSE;
        }

        Glib::ustring myDisplayValue;
        if ( ! convertFrom(cx, argv[1], myDisplayValue)) {
            JS_ReportError(cx, "TNTMeasurementList::append(): Argument 1 must be a string.");
            return JS_FALSE;
        }

        Ptr<Gtk::TreeIter> myIt = Ptr<Gtk::TreeIter>( new Gtk::TreeIter(
                            myNative->append(myMeasurementNode, myDisplayValue)));
        *rval = as_jsval(cx, myIt, &(*myIt));
        return JS_TRUE;

    } HANDLE_CPP_EXCEPTION;
}

static JSBool
clear(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    
    try {
        ensureParamCount(argc, 0);

        acgtk::TNTMeasurementList * myNative;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        
        myNative->clear();
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}


JSFunctionSpec *
JSTNTMeasurementList::Functions() {
    IF_REG(cerr << "Registering class '"<<ClassName()<<"'"<<endl);
    static JSFunctionSpec myFunctions[] = {
        // name                      native                    nargs
        {"toString",                 toString,                 0},
        {"registerTypeIcon",         registerTypeIcon,         2},
        {"append",                   append,                   2},
        {"clear",                    clear,                    0},
        {0}
    };
    return myFunctions;
}

JSPropertySpec *
JSTNTMeasurementList::Properties() {
    static JSPropertySpec myProperties[] = {
        {"signal_visible_toggled", PROP_signal_visible_toggled, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"signal_editable_toggled", PROP_signal_editable_toggled, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {"signal_right_click", PROP_signal_right_click, JSPROP_READONLY|JSPROP_ENUMERATE|JSPROP_PERMANENT},
        {0}
    };
    return myProperties;
}

// getproperty handling
JSBool
JSTNTMeasurementList::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return getPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSTNTMeasurementList::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    JSClassTraits<NATIVE>::ScopedNativeRef myObj(cx, obj);
    return setPropertySwitch(myObj.getNative(), theID, cx, obj, id, vp);
}

JSBool
JSTNTMeasurementList::getPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case PROP_signal_visible_toggled:
            {
                JSSignal1<void, Glib::ustring>::OWNERPTR mySignal( new
                        JSSignal1<void, Glib::ustring>::NATIVE(theNative.signal_visible_toggled()));
                *vp = jslib::as_jsval(cx, mySignal);
                return JS_TRUE;
            }
        case PROP_signal_editable_toggled:
            {
                JSSignal1<void, Glib::ustring>::OWNERPTR mySignal( new
                        JSSignal1<void, Glib::ustring>::NATIVE(theNative.signal_editable_toggled()));
                *vp = jslib::as_jsval(cx, mySignal);
                return JS_TRUE;
            }
        case PROP_signal_right_click:
            {
                JSSignal0<void>::OWNERPTR mySignal( new
                        JSSignal0<void>::NATIVE(theNative.signal_right_click()));
                *vp = jslib::as_jsval(cx, mySignal);
                return JS_TRUE;
            }
        default:
            return JSBASE::getPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}
JSBool
JSTNTMeasurementList::setPropertySwitch(NATIVE & theNative, unsigned long theID,
        JSContext *cx, JSObject *obj, jsval id, jsval *vp)
{
    switch (theID) {
        case 0:
        default:
            return JSBASE::setPropertySwitch(theNative, theID, cx, obj, id, vp);
    }
}

JSBool
JSTNTMeasurementList::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }

    NATIVE * newNative = 0;

    JSTNTMeasurementList * myNewObject = 0;

    try {
        if (argc == 0) {
            newNative = new NATIVE();
        } else if (argc == 4) {
            std::string myVisibleIcon;
            if ( ! convertFrom(cx, argv[0], myVisibleIcon)) {
                JS_ReportError(cx, "TNTMeasurementList::Constructor(): Argument 0 must be a string.");
                return JS_FALSE;
            }
            std::string myVisibleIconWP = searchFileRelativeToJSInclude(cx, obj, argc, argv, myVisibleIcon);

            std::string myHiddenIcon;
            if ( ! convertFrom(cx, argv[1], myHiddenIcon)) {
                JS_ReportError(cx, "TNTMeasurementList::Constructor(): Argument 1 must be a string.");
                return JS_FALSE;
            }
            std::string myHiddenIconWP = searchFileRelativeToJSInclude(cx, obj, argc, argv, myHiddenIcon);

            std::string myLockedIcon;
            if ( ! convertFrom(cx, argv[2], myLockedIcon)) {
                JS_ReportError(cx, "TNTMeasurementList::Constructor(): Argument 2 must be a string.");
                return JS_FALSE;
            }
            std::string myLockedIconWP = searchFileRelativeToJSInclude(cx, obj, argc, argv, myLockedIcon);

            std::string myEditableIcon;
            if ( ! convertFrom(cx, argv[3], myEditableIcon)) {
                JS_ReportError(cx, "TNTMeasurementList::Constructor(): Argument 3 must be a string.");
                return JS_FALSE;
            }
            std::string myEditableIconWP = searchFileRelativeToJSInclude(cx, obj, argc, argv, myEditableIcon);

            newNative = new NATIVE(myVisibleIconWP, myHiddenIconWP, myLockedIconWP, myEditableIconWP);
        } else {
            JS_ReportError(cx,"Constructor for %s: bad number of arguments: expected none () %d",ClassName(), argc);
            return JS_FALSE;
        }

        myNewObject = new JSTNTMeasurementList(OWNERPTR(newNative), newNative);

        if (myNewObject) {
            JS_SetPrivate(cx,obj,myNewObject);
            return JS_TRUE;
        }
    } HANDLE_CPP_EXCEPTION;
    JS_ReportError(cx,"JSTNTMeasurementList::Constructor: bad parameters");
    return JS_FALSE;
}

enum ColumnProps {
    PROP_COL_IS_VISIBLE,
    PROP_COL_TYPE_ICON,
    PROP_COL_NAME,
    PROP_COL_VALUE,
    PROP_COL_IS_EDITABLE,
    PROP_COL_XML_ID,
    PROP_COL_TYPE
};

#define DEFINE_COL_PROP(theName) \
    {"COL_" #theName, PROP_COL_ ## theName, acgtk::TNTMeasurementList::COL_ ## theName}
JSConstIntPropertySpec *
JSTNTMeasurementList::ConstIntProperties() {

    static JSConstIntPropertySpec myProperties[] = {
        DEFINE_COL_PROP(IS_VISIBLE),
        DEFINE_COL_PROP(TYPE_ICON),
        DEFINE_COL_PROP(NAME),
        DEFINE_COL_PROP(VALUE),
        DEFINE_COL_PROP(IS_EDITABLE),
        DEFINE_COL_PROP(XML_ID),
        DEFINE_COL_PROP(TYPE),
        {0}
    };
    return myProperties;
};

void
JSTNTMeasurementList::addClassProperties(JSContext * cx, JSObject * theClassProto) {
    JSBASE::addClassProperties(cx, theClassProto);
    JSA_AddFunctions(cx, theClassProto, Functions());
    JSA_AddProperties(cx, theClassProto, Properties());
    createClassModuleDocumentation("gtk", ClassName(), Properties(), Functions(),
            ConstIntProperties(), 0, 0, JSBASE::ClassName());
}

JSObject *
JSTNTMeasurementList::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject * myClassObject = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, 0,0);
    if (myClassObject) {
        addClassProperties(cx, myClassObject);
    }
    jsval myConstructorFuncObjVal;
    if (JS_GetProperty(cx, theGlobalObject, ClassName(), &myConstructorFuncObjVal)) {
        JSObject * myConstructorFuncObj = JSVAL_TO_OBJECT(myConstructorFuncObjVal);
        JSA_DefineConstInts(cx, myConstructorFuncObj, ConstIntProperties());
    } else {
        cerr << "JSTNTMeasurementList::initClass: constructor function object not found, could not initialize static members"<<endl;
    }
    return myClassObject;
}

jsval as_jsval(JSContext *cx, JSTNTMeasurementList::OWNERPTR theOwner, JSTNTMeasurementList::NATIVE * theNative) {
    JSObject * myReturnObject = JSTNTMeasurementList::Construct(cx, theOwner, theNative);
    return OBJECT_TO_JSVAL(myReturnObject);
}

}

