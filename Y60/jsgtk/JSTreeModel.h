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
//   $RCSfile: JSTreeModel.h,v $
//   $Author: martin $
//   $Revision: 1.4 $
//   $Date: 2004/11/27 16:22:00 $
//
//
//=============================================================================

#ifndef _Y60_ACGTKSHELL_JSTREEMODEL_INCLUDED_
#define _Y60_ACGTKSHELL_JSTREEMODEL_INCLUDED_

#include <y60/JSWrapper.h>
#include <gtkmm/treemodel.h>

#include <asl/string_functions.h>

namespace jslib {

class JSTreeModel : public JSWrapper<Gtk::TreeModel, Glib::RefPtr<Gtk::TreeModel>, StaticAccessProtocol> {
        JSTreeModel();  // hide default constructor
    public:
        virtual ~JSTreeModel() {
        }
        typedef Gtk::TreeModel NATIVE;
        typedef Glib::RefPtr<Gtk::TreeModel> OWNERPTR;
        typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

        static const char * ClassName() {
            return "TreeModel";
        }

        static JSFunctionSpec * Functions();

        enum PropertyNumbers {
            PROP_children_size = -100,
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

        JSTreeModel(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        {}

        // static JSConstIntPropertySpec * ConstIntProperties();
        static void addClassProperties(JSContext * cx, JSObject * theClassProto);
        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);

        static JSTreeModel & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSTreeModel &>(JSTreeModel::getJSWrapper(cx,obj));
        }
    private:

};

template <>
struct JSClassTraits<JSTreeModel::NATIVE>
    : public JSClassTraitsWrapper<JSTreeModel::NATIVE, JSTreeModel> {};

jsval as_jsval(JSContext *cx, JSTreeModel::OWNERPTR theOwner, JSTreeModel::NATIVE * theTreeModel);

}

#endif



