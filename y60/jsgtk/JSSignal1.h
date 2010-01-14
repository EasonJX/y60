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

#ifndef _Y60_ACGTKSHELL_JSSIGNAL_1_INCLUDED_
#define _Y60_ACGTKSHELL_JSSIGNAL_1_INCLUDED_

#include "y60_jsgtk_settings.h"

#include "JSSignalProxyUtils.h"
#include "jsgtk.h"
#include "JSSigConnection.h"
#include <y60/acgtk/GCObserver.h>
#include <y60/jsbase/JSWrapper.h>
#include <libglademm/xml.h>
#include <sigc++/slot.h>
#include <sigc++/connection.h>

#include <asl/base/string_functions.h>

namespace jslib {

template <class R, class P0>
bool convertFrom(JSContext *cx, jsval theValue, asl::Ptr<sigc::signal1<R, P0> > & theOwner);

template <class R, class P0>
class JSSignalAdapter1 : public JSSignalAdapterBase {
    public:
        static R on_signal(P0 theParam0,
                           JSContext * cx, JSObject * theJSObject, Glib::ustring theMethodName)
        {
            // call the function
            jsval argv[1], rval;
            argv[0] = as_jsval(cx, theParam0);
            JSBool ok = jslib::JSA_CallFunctionName(cx, theJSObject, theMethodName, 1, argv, &rval);
            (void)(ok); //XXX check if caller will correctly propagate JS exceptions

            R myResult;
            convertFrom(cx, rval, myResult);
            return myResult;
        }
};

template <class P0>
class JSSignalAdapter1<void, P0> : public JSSignalAdapterBase {
    public:
        static void on_signal(P0 theParam0,
                              JSContext * cx,  JSObject * theJSObject, Glib::ustring theMethodName)
        {
            // call the function
            jsval argv[1], rval;
            argv[0] = as_jsval(cx, theParam0);
            JSBool ok = jslib::JSA_CallFunctionName(cx, theJSObject, theMethodName, 1, argv, &rval);
            (void)(ok); //XXX check if caller will correctly propagate JS exceptions
        }
};

template <class R, class P0>
std::string
composeTypeName(const char * theBasename) {
    return std::string(theBasename) + "_" + TypeNameTrait<R>::name() + "_" +
                                            TypeNameTrait<P0>::name() + "_";
}

template <class R, class P0>
class JSSignal1 : public JSWrapper<sigc::signal1<R, P0>, asl::Ptr<sigc::signal1<R, P0> >, StaticAccessProtocol>
{
        JSSignal1() {}
    public:
        typedef sigc::signal1<R, P0> NATIVE;
        typedef asl::Ptr<NATIVE> OWNERPTR;
        typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

        static const char * ClassName() {
            static std::string myName(composeTypeName<R,P0>("Signal1"));
            return myName.c_str();
        }

        static JSBool connect(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
            try {
                ensureParamCount(argc, 2);
                OWNERPTR myOwner;
                convertFrom(cx, OBJECT_TO_JSVAL(obj), myOwner);

                JSObject * myTarget(0);
                convertFrom(cx, argv[0], myTarget);

                Glib::ustring myMethodName;
                convertFrom(cx, argv[1], myMethodName);

                sigc::slot1<R, P0> mySlot = sigc::bind<JSContext*, JSObject*, Glib::ustring>(
                    sigc::ptr_fun( & JSSignalAdapter1<R, P0>::on_signal ), cx, myTarget, myMethodName);
                JSSigConnection::OWNERPTR myConnection = JSSigConnection::OWNERPTR(new sigc::connection);

                *myConnection = myOwner->connect(mySlot);

                // register our target object with the GCObserver
                GCObserver::FinalizeSignal myFinalizer = GCObserver::get().watchObject(myTarget);
                // now add our cleanup code to the finalize signal,
                // binding the connection as an extra argument
                myFinalizer.connect(sigc::bind<sigc::connection>(
                            sigc::ptr_fun( & JSSignalAdapter1<R, P0>::on_target_finalized),
                            *myConnection));
                *rval = as_jsval(cx, myConnection, & ( * myConnection));
                return JS_TRUE;
            } HANDLE_CPP_EXCEPTION;
        }

        static JSFunctionSpec * Functions() {
            static JSFunctionSpec myFunctions[] = {
                {"connect", connect, 2},
                {0}
            };
            return myFunctions;
        }

        virtual unsigned long length() const {
            return 1;
        }

        // virtual JSBool getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);
        //virtual JSBool setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);

        static JSBool
        Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
            if (JSA_GetClass(cx,obj) != Base::Class()) {
                JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
                return JS_FALSE;
            }

            NATIVE * newNative = 0;
            JSSignal1<R, P0> * myNewObject = 0;

            if (argc == 0) {
                myNewObject = new JSSignal1<R, P0>(OWNERPTR(newNative), newNative);
            } else {
                JS_ReportError(cx,"Constructor for %s: bad number of arguments: expected none () %d",ClassName(), argc);
                return JS_FALSE;
            }

            if (myNewObject) {
                JS_SetPrivate(cx,obj,myNewObject);
                return JS_TRUE;
            }
            JS_ReportError(cx,"JSWindow::Constructor: bad parameters");
            return JS_FALSE;
        }


        static
        JSObject * Construct(JSContext *cx, OWNERPTR theOwner) {
            return Base::Construct(cx, theOwner, theOwner.get());
        }

        JSSignal1(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        {}

        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject) {
            JSObject * myClassObject = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, 0,0);
            if (myClassObject) {
                JSA_AddFunctions(cx, myClassObject, Functions());
            }
            return myClassObject;
        }

        static JSSignal1<R, P0> & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSSignal1<R, P0> &>(JSSignal1<R, P0>::getJSWrapper(cx,obj));
        }

    private:
        static JSBool
        toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
            DOC_BEGIN("");
            DOC_END;
            std::string myStringRep = std::string("Signal1@") + asl::as_string(obj);
            *rval = as_jsval(cx, myStringRep);
            return JS_TRUE;
        }
};

#define DEFINE_SIGNAL1_CLASS_TRAIT(R, P0) \
template <> \
struct JSClassTraits<JSSignal1<R, P0>::NATIVE> \
    : public JSClassTraitsWrapper<JSSignal1<R, P0>::NATIVE, JSSignal1<R, P0> > {};

#define INIT_SIGNAL1( RVAL, PARAM0 ) \
if (!JSSignal1<RVAL, PARAM0>::initClass(cx, theGlobalObject)) { \
    return false; \
}

template <class R, class P0>
jsval as_jsval(JSContext *cx, asl::Ptr<sigc::signal1<R, P0> > & theValue) {
    JSObject * myObject = JSSignal1<R, P0>::Construct(cx, theValue);
    return OBJECT_TO_JSVAL(myObject);
}

}

#endif


