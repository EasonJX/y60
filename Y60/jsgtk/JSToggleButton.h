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
//   $RCSfile: JSToggleButton.h,v $
//   $Author: martin $
//   $Revision: 1.2 $
//   $Date: 2004/11/27 16:22:00 $
//
//
//=============================================================================

#ifndef _Y60_ACGTKSHELL_JSTOGGLEBUTTON_INCLUDED_
#define _Y60_ACGTKSHELL_JSTOGGLEBUTTON_INCLUDED_

#include "JSButton.h"
#include <y60/JSWrapper.h>
#include <gtkmm/togglebutton.h>

#include <asl/string_functions.h>

namespace jslib {

class JSToggleButton : public JSWrapper<Gtk::ToggleButton, asl::Ptr<Gtk::ToggleButton>, StaticAccessProtocol> {
        JSToggleButton();  // hide default constructor
    public:
        typedef jslib::JSButton JSBASE;
        virtual ~JSToggleButton() {
        }
        typedef Gtk::ToggleButton NATIVE;
        typedef asl::Ptr<Gtk::ToggleButton> OWNERPTR;
        typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

        static const char * ClassName() {
            return "ToggleButton";
        };

        static JSFunctionSpec * Functions();

        enum PropertyNumbers {
             PROP_active = JSBASE::PROP_END,
             PROP_signal_toggled,
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

        JSToggleButton(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        { }

        static JSConstIntPropertySpec * ConstIntProperties();
        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);
        static void addClassProperties(JSContext * cx, JSObject * theClassProto);

        static JSToggleButton & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSToggleButton &>(JSToggleButton::getJSWrapper(cx,obj));
        }

    private:
};

template <>
struct JSClassTraits<JSToggleButton::NATIVE>
    : public JSClassTraitsWrapper<JSToggleButton::NATIVE, JSToggleButton> {};

jsval as_jsval(JSContext *cx, JSToggleButton::OWNERPTR theOwner, JSToggleButton::NATIVE * theToggleButton);

} // namespace

#endif




