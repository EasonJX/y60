//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _Y60_ACGTKSHELL_JSRANGE_INCLUDED_
#define _Y60_ACGTKSHELL_JSRANGE_INCLUDED_

#include "JSWidget.h"

#include <y60/jsbase/JSWrapper.h>

#if defined(_MSC_VER)
#pragma warning(push,1)
#endif //defined(_MSC_VER)
#include <gtkmm/range.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif //defined(_MSC_VER)

#include <asl/base/string_functions.h>

namespace jslib {

class JSRange : public JSWrapper<Gtk::Range, asl::Ptr<Gtk::Range>, StaticAccessProtocol> {
    private:
        JSRange();  // hide default constructor
        typedef JSWidget JSBASE;
    public:
        virtual ~JSRange() {
        }
        typedef Gtk::Range NATIVE;
        typedef asl::Ptr<Gtk::Range> OWNERPTR;
        typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

        static const char * ClassName() {
            return "Range";
        };

        static JSFunctionSpec * Functions();

        enum PropertyNumbers {
            PROP_adjustment = JSBASE::PROP_END,
            PROP_inverted,
            PROP_update_policy,
            PROP_value,
            PROP_signal_adjust_bounds,
            PROP_signal_move_slider,
            PROP_signal_value_changed,
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

        JSRange(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        {}

        static JSConstIntPropertySpec * ConstIntProperties();
        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);
        static void addClassProperties(JSContext * cx, JSObject * theClassProto);

        static JSRange & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSRange &>(JSRange::getJSWrapper(cx,obj));
        }

    private:
};


template <>
struct JSClassTraits<JSRange::NATIVE>
    : public JSClassTraitsWrapper<JSRange::NATIVE, JSRange> {};

jsval as_jsval(JSContext *cx, JSRange::OWNERPTR theOwner, JSRange::NATIVE * theRange);

} // namespace

#endif

