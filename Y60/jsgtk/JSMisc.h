//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _Y60_ACGTKSHELL_JSMISC_INCLUDED_
#define _Y60_ACGTKSHELL_JSMISC_INCLUDED_

#include "JSWidget.h"

#include <y60/JSWrapper.h>
#include <gtkmm/misc.h>

#include <asl/string_functions.h>

namespace jslib {

class JSMisc : public JSWrapper<Gtk::Misc, asl::Ptr<Gtk::Misc>, StaticAccessProtocol> {
    private:
        JSMisc();  // hide default constructor
        typedef JSWidget JSBASE;
    public:
        virtual ~JSMisc() {
        }
        typedef Gtk::Misc NATIVE;
        typedef asl::Ptr<Gtk::Misc> OWNERPTR;
        typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

        static const char * ClassName() {
            return "Misc";
        };

        static JSFunctionSpec * Functions();

        enum PropertyNumbers {
            PROP_alignment = JSBASE::PROP_END,
            PROP_padding,
            PROP_END
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

        JSMisc(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        {}

        static JSConstIntPropertySpec * ConstIntProperties();
        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);
        static void addClassProperties(JSContext * cx, JSObject * theClassProto);

        static JSMisc & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSMisc &>(JSMisc::getJSWrapper(cx,obj));
        }

    private:
};


template <>
struct JSClassTraits<JSMisc::NATIVE>
    : public JSClassTraitsWrapper<JSMisc::NATIVE, JSMisc> {};

jsval as_jsval(JSContext *cx, JSMisc::OWNERPTR theOwner, JSMisc::NATIVE * theMisc);

} // namespace

#endif

