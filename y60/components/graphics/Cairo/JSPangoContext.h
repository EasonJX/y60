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
*/

#ifndef _Y60_PANGO_JSCONTEXT_INCLUDED_
#define _Y60_PANGO_JSCONTEXT_INCLUDED_

#include <y60/jsbase/JSWrapper.h>

#include <pango/pango-context.h>

namespace jslib {

    namespace pango {
        // a quick wrapper to ensure the correct *_free is called on destruction
        class ContextWrapper {
            public:
                // note: some pango calls increase the refcount for you (e.g. pango_context_new),
                // some don't (pango_layout_get_context). In the first case, we must set theIncRefcountFlag to false,
                // in the latter case to true.
                ContextWrapper(PangoContext * theNative, bool theIncRefcountFlag) :
                    _myNative(theNative) 
                {
                    if (theIncRefcountFlag) {
                        g_object_ref(_myNative);
                    }
                };
                ~ContextWrapper() {
                    if (_myNative) {
                        g_object_unref(_myNative);
                    }
                }
                PangoContext * get() const { return _myNative; };
            private:
                // not copy-safe: hide copy ctor & assignment op
                ContextWrapper(const ContextWrapper &);
                ContextWrapper & operator=(const ContextWrapper &);
                PangoContext * _myNative;
                
        };
        class JSContext : public JSWrapper<ContextWrapper, asl::Ptr< ContextWrapper >, StaticAccessProtocol> {
            JSContext();  // hide default constructor
            public:

            virtual ~JSContext() {
            }

            typedef ContextWrapper NATIVE;
            typedef asl::Ptr< ContextWrapper > OWNERPTR;

            typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

            static const char * ClassName() {
                return "Context";
            }

            static JSFunctionSpec * Functions();

            enum PropertyNumbers {
                PROP_END
            };
            static JSPropertySpec * Properties();

            virtual unsigned long length() const {
                return 1;
            }

            static JSBool getPropertySwitch(NATIVE & theNative, unsigned long theID,
                    ::JSContext *cx, JSObject *obj, jsval id, jsval *vp);
            virtual JSBool getPropertySwitch(unsigned long theID, ::JSContext *cx, JSObject *obj, jsval id, jsval *vp);
            static JSBool setPropertySwitch(NATIVE & theNative, unsigned long theID, ::JSContext *cx,
                    JSObject *obj, jsval id, jsval *vp);
            virtual JSBool setPropertySwitch(unsigned long theID, ::JSContext *cx, JSObject *obj, jsval id, jsval *vp);

            static JSBool
                Constructor(::JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

            static
                JSObject * Construct(::JSContext *cx, OWNERPTR theOwner, NATIVE * theNative) {
                    return Base::Construct(cx, theOwner, theNative);
                }

            JSContext(OWNERPTR theOwner, NATIVE * theNative)
                : Base(theOwner, theNative)
            { }

            static JSConstIntPropertySpec * ConstIntProperties();
            static JSObject * initClass(::JSContext *cx, JSObject *theGlobalObject);
            static void addClassProperties(::JSContext * cx, JSObject * theClassProto);

            static JSContext & getObject(::JSContext *cx, JSObject * obj) {
                return dynamic_cast<JSContext &>(JSContext::getJSWrapper(cx,obj));
            }

        };
    };

    template <>
    struct JSClassTraits<pango::JSContext::NATIVE>
        : public JSClassTraitsWrapper<pango::JSContext::NATIVE, pango::JSContext> {};

    jsval as_jsval(JSContext *cx, pango::JSContext::OWNERPTR theOwner, pango::JSContext::NATIVE * theNative);

    bool convertFrom(JSContext *cx, jsval theValue, pango::JSContext::OWNERPTR & theOwner);
}

#endif
