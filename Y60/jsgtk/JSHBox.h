//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _Y60_ACGTKSHELL_JSHBOX_INCLUDED_
#define _Y60_ACGTKSHELL_JSHBOX_INCLUDED_

#include "JSBox.h"
#include <y60/JSWrapper.h>
#include <gtkmm/box.h>

#include <asl/string_functions.h>

namespace jslib {

class JSHBox : public JSWrapper<Gtk::HBox, asl::Ptr<Gtk::HBox>, StaticAccessProtocol> {
    private:
        JSHBox();  // hide default constructor
        typedef JSBox JSBASE;
    public:
        virtual ~JSHBox() {
        }
        typedef Gtk::HBox NATIVE;
        typedef asl::Ptr<Gtk::HBox> OWNERPTR;
        typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

        static const char * ClassName() {
            return "HBox";
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

        JSHBox(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        { }

        static JSConstIntPropertySpec * ConstIntProperties();
        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);
        static void addClassProperties(JSContext * cx, JSObject * theClassProto);

        static JSHBox & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSHBox &>(JSHBox::getJSWrapper(cx,obj));
        }

    private:
};

template <>
struct JSClassTraits<JSHBox::NATIVE>
    : public JSClassTraitsWrapper<JSHBox::NATIVE, JSHBox> {};

jsval as_jsval(JSContext *cx, JSHBox::OWNERPTR theOwner, JSHBox::NATIVE * theHBox);

} // namespace

#endif

