//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _Y60_ACGTKSHELL_JSHRULER_INCLUDED_
#define _Y60_ACGTKSHELL_JSHRULER_INCLUDED_

#include "JSRuler.h"
#include <y60/jsbase/JSWrapper.h>

#include <asl/base/string_functions.h>

namespace jslib {

class JSHRuler : public JSWrapper<Gtk::HRuler, asl::Ptr<Gtk::HRuler>, StaticAccessProtocol> {
    private:
        JSHRuler();  // hide default constructor
        typedef JSRuler JSBASE;
    public:
        virtual ~JSHRuler() {
        }
        typedef Gtk::HRuler NATIVE;
        typedef asl::Ptr<Gtk::HRuler> OWNERPTR;
        typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

        static const char * ClassName() {
            return "HRuler";
        };
        static JSFunctionSpec * Functions();

        enum PropertyNumbers {
            PROP_END = JSBASE::PROP_END
        };
        static JSPropertySpec * Properties();

        virtual unsigned long length() const {
            return 1;
        }

        static JSBool getPropertySwitch(NATIVE & theNative, unsigned long theID,
                JSContext *cx, JSObject *obj, jsval id, jsval *vp);
        virtual JSBool getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);
        static JSBool setPropertySwitch(NATIVE & theNative, unsigned long theID, JSContext *cx,
                JSObject *obj, jsval id, jsval *vp);
        virtual JSBool setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);

        static JSBool
        Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

        static
        JSObject * Construct(JSContext *cx, OWNERPTR theOwner, NATIVE * theNative) {
            return Base::Construct(cx, theOwner, theNative);
        }

        JSHRuler(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        {}

        static JSConstIntPropertySpec * ConstIntProperties();
        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);
        static void addClassProperties(JSContext * cx, JSObject * theClassProto);

        static JSHRuler & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSHRuler &>(JSHRuler::getJSWrapper(cx,obj));
        }

    private:
};

template <>
struct JSClassTraits<JSHRuler::NATIVE>
    : public JSClassTraitsWrapper<JSHRuler::NATIVE, JSHRuler> {};

jsval as_jsval(JSContext *cx, JSHRuler::OWNERPTR theOwner, JSHRuler::NATIVE * theHRuler);

} // namespace

#endif

