//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _Y60_ACGTKSHELL_JSSIGNAL_2_INCLUDED_
#define _Y60_ACGTKSHELL_JSSIGNAL_2_INCLUDED_

#include "JSSignalProxyUtils.h"
#include "JSSigConnection.h"
#include "jsgtk.h"
#include <y60/acgtk/GCObserver.h>
#include <y60/jsbase/JSWrapper.h>
#include <libglademm/xml.h>
#include <sigc++/slot.h>
#include <sigc++/connection.h>

#include <asl/base/string_functions.h>

namespace jslib {

template <class R, class P0, class P1>
bool convertFrom(JSContext *cx, jsval theValue, asl::Ptr<sigc::signal2<R, P0, P1> > & theOwner);

template <class R, class P0, class P1>
class JSSignalAdapter2 : public JSSignalAdapterBase {
    public:
        static R on_signal(P0 theParam0, P1 theParam1,
                           JSContext * cx, JSObject * theJSObject, Glib::ustring theMethodName)
        {
            // call the function
            jsval argv[2], rval;
            argv[0] = as_jsval(cx, theParam0);
            argv[1] = as_jsval(cx, theParam1);
            AC_TRACE << "calling signal2 " << theMethodName;
            JSBool ok = jslib::JSA_CallFunctionName(cx, theJSObject, theMethodName, 2, argv, &rval);

            R myResult;
            convertFrom(cx, rval, myResult);
            return myResult;
        }
};

template <class P0, class P1>
class JSSignalAdapter2<void, P0, P1> : public JSSignalAdapterBase {
    public:
        static void on_signal(P0 theParam0, P1 theParam1,
                              JSContext * cx,  JSObject * theJSObject, Glib::ustring theMethodName)
        {
            jsval argv[2], rval;
            argv[0] = as_jsval(cx, theParam0);
            argv[1] = as_jsval(cx, theParam1);
            JSBool ok = jslib::JSA_CallFunctionName(cx, theJSObject, theMethodName, 2, argv, &rval);
        }
};

template <class R, class P0, class P1>
std::string
composeTypeName(const char * theBasename) {
    return std::string(theBasename) + "_" + TypeNameTrait<R>::name() + "_" +
                                            TypeNameTrait<P0>::name() + "_" +
                                            TypeNameTrait<P1>::name() + "_";
}

template <class R, class P0, class P1>
class JSSignal2 : public JSWrapper<sigc::signal2<R, P0, P1>, asl::Ptr<sigc::signal2<R, P0, P1> >, StaticAccessProtocol>
{
        JSSignal2() {}
    public:
        typedef sigc::signal2<R, P0, P1> NATIVE;
        typedef asl::Ptr<NATIVE> OWNERPTR;
        typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

        static const char * ClassName() {
            static std::string myName(composeTypeName<R,P0,P1>("Signal2"));
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

                sigc::slot2<R, P0, P1> mySlot = sigc::bind<JSContext*, JSObject*, Glib::ustring>(
                    sigc::ptr_fun( & JSSignalAdapter2<R, P0, P1>::on_signal ), cx, myTarget, myMethodName);
                JSSigConnection::OWNERPTR myConnection = JSSigConnection::OWNERPTR(new sigc::connection);

                *myConnection = myOwner->connect(mySlot);

                // register our target object with the GCObserver
                GCObserver::FinalizeSignal myFinalizer = GCObserver::get().watchObject(myTarget);
                // now add our cleanup code to the finalize signal,
                // binding the connection as an extra argument
                myFinalizer.connect(sigc::bind<sigc::connection>(
                            sigc::ptr_fun( & JSSignalAdapter2<R, P0, P1>::on_target_finalized),
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
            JSSignal2<R, P0, P1> * myNewObject = 0;

            if (argc == 0) {
                myNewObject = new JSSignal2<R, P0, P1>(OWNERPTR(newNative), newNative);
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
            return Base::Construct(cx, theOwner, &(*theOwner));
        }

        JSSignal2(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        {}

        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject) {
            JSObject * myClassObject = Base::initClass(cx, theGlobalObject, ClassName(), Constructor, 0,0);
            if (myClassObject) {
                JSA_AddFunctions(cx, myClassObject, Functions());
            }
            return myClassObject;
        }

        static JSSignal2<R, P0, P1> & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSSignal2<R, P0, P1> &>(JSSignal2<R, P0, P1>::getJSWrapper(cx,obj));
        }

    private:
        static JSBool
        toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
            DOC_BEGIN("");
            DOC_END;
            std::string myStringRep = std::string("Signal2@") + asl::as_string(obj);
            *rval = as_jsval(cx, myStringRep);
            return JS_TRUE;
        }
};

#define DEFINE_SIGNAL2_CLASS_TRAIT(R, P0, P1) \
template <> \
struct JSClassTraits<JSSignal2<R, P0, P1>::NATIVE> \
    : public JSClassTraitsWrapper<JSSignal2<R, P0, P1>::NATIVE, JSSignal2<R, P0, P1> > {};

#define INIT_SIGNAL2( RVAL, PARAM0, PARAM1 ) \
if (!JSSignal2<RVAL, PARAM0, PARAM1>::initClass(cx, theGlobalObject)) { \
    return false; \
}

template <class R, class P0, class P1>
jsval as_jsval(JSContext *cx, asl::Ptr<sigc::signal2<R, P0, P1> > & theValue) {
    JSObject * myObject = JSSignal2<R, P0, P1>::Construct(cx, theValue);
    return OBJECT_TO_JSVAL(myObject);
}

}
#endif


