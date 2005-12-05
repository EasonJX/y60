//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: JSToolItem.h,v $
//   $Author: martin $
//   $Revision: 1.2 $
//   $Date: 2004/11/27 16:22:00 $
//
//
//=============================================================================

#ifndef _Y60_ACGTKSHELL_JSTOOLITEM_INCLUDED_
#define _Y60_ACGTKSHELL_JSTOOLITEM_INCLUDED_

#include "JSBin.h"
#include <y60/JSWrapper.h>
#include <gtkmm/toolitem.h>

#include <asl/string_functions.h>

namespace jslib {

class JSToolItem : public JSWrapper<Gtk::ToolItem, asl::Ptr<Gtk::ToolItem>, StaticAccessProtocol> {
        JSToolItem();  // hide default constructor
    typedef JSBin JSBASE;
    public:
        virtual ~JSToolItem() {
        }
        typedef Gtk::ToolItem NATIVE;
        typedef asl::Ptr<Gtk::ToolItem> OWNERPTR;
        typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

        static const char * ClassName() {
            return "ToolItem";
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

        JSToolItem(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        { }

        static JSConstIntPropertySpec * ConstIntProperties();
        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);
        static void addClassProperties(JSContext * cx, JSObject * theClassProto);

        static JSToolItem & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSToolItem &>(JSToolItem::getJSWrapper(cx,obj));
        }

    private:
};

template <>
struct JSClassTraits<JSToolItem::NATIVE>
    : public JSClassTraitsWrapper<JSToolItem::NATIVE, JSToolItem> {};

jsval as_jsval(JSContext *cx, JSToolItem::OWNERPTR theOwner, JSToolItem::NATIVE * theToolItem);

} // namespace

#endif



