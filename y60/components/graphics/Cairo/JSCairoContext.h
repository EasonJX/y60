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

#ifndef _Y60_CAIRO_JSCAIRO_INCLUDED_
#define _Y60_CAIRO_JSCAIRO_INCLUDED_

#include <asl/dom/Nodes.h>

#include <y60/jsbase/JSWrapper.h>

extern "C" {
#include <cairo.h>
}

#include "CairoWrapper.h"

namespace jslib {

    typedef CairoWrapper<cairo_t> JSCairoWrapper;

    namespace cairo {

        class JSContext : public JSWrapper<JSCairoWrapper, asl::Ptr< JSCairoWrapper >, StaticAccessProtocol> {
            JSContext();  // hide default constructor
            public:

            virtual ~JSContext() {
            }

            typedef JSCairoWrapper NATIVE;
            typedef asl::Ptr< JSCairoWrapper > OWNERPTR;

            typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

            static const char * ClassName() {
                return "Context";
            }

            static JSFunctionSpec * Functions();

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
    struct JSClassTraits<cairo::JSContext::NATIVE>
        : public JSClassTraitsWrapper<cairo::JSContext::NATIVE, cairo::JSContext> {};

    jsval as_jsval(JSContext *cx, cairo::JSContext::OWNERPTR theOwner, cairo::JSContext::NATIVE * theContext);
    jsval as_jsval(JSContext *cx, cairo::JSContext::OWNERPTR theOwner, cairo_t * theContext);

    bool convertFrom(JSContext *cx, jsval theValue, cairo::JSContext::NATIVE *& theContext);
    bool convertFrom(JSContext *cx, jsval theValue, cairo_t *& theContext);
}

#endif /* !_Y60_CAIRO_JSCAIROCONTEXT_INCLUDED_ */
