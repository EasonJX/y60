//============================================================================
//
// Copyright (C) 2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================
//
//   $RCSfile: $
//
//   $Author: $
//
//   $Revision: $
//
//=============================================================================

#ifndef _ac_jslib_JSOffScreenRenderArea_h_
#define _ac_jslib_JSOffScreenRenderArea_h_

#include "OffScreenRenderArea.h"
#include "JSAbstractRenderWindow.h"

/**
 * 
 * @ingroup Y60jslib
 */ 
namespace jslib {

class JSOffScreenRenderArea : public jslib::JSWrapper<jslib::OffScreenRenderArea, asl::Ptr<OffScreenRenderArea>, StaticAccessProtocol> {
    public:
        typedef jslib::OffScreenRenderArea NATIVE;
        typedef asl::Ptr<NATIVE> OWNERPTR;
        typedef JSWrapper<NATIVE,OWNERPTR,StaticAccessProtocol> Base;
        typedef JSAbstractRenderWindow<jslib::OffScreenRenderArea> JSBASE;
        
        static const char * ClassName() {
            return "OffScreenRenderArea";
        }
        static JSFunctionSpec * Functions();
        static JSPropertySpec * Properties();
        static JSConstIntPropertySpec * ConstIntProperties();
        static JSPropertySpec * StaticProperties();
        static JSFunctionSpec * StaticFunctions();
        
        static void addClassProperties(JSContext * cx, JSObject * theClassProto);
        virtual unsigned long length() const {
            return 1;
        }

        static JSBool getPropertySwitch(NATIVE & theNative, unsigned long theID,
                                        JSContext *cx, JSObject *obj, jsval id, jsval *vp);
        virtual JSBool getPropertySwitch(unsigned long theID, JSContext *cx,
                                         JSObject *obj, jsval id, jsval *vp);
        static JSBool setPropertySwitch(NATIVE & theNative, unsigned long theID,
                                        JSContext *cx, JSObject *obj, jsval id, jsval *vp);
        virtual JSBool setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj,
                                         jsval id, jsval *vp);

        static JSBool
        Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

        static
        JSObject * Construct(JSContext *cx, OWNERPTR theOwner, NATIVE * theNative) {
            return Base::Construct(cx, theOwner, theNative);
        }

        JSOffScreenRenderArea(OWNERPTR theOwner, NATIVE * theNative)
            : Base(theOwner, theNative)
        {}

        static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);

        static JSOffScreenRenderArea & getObject(JSContext *cx, JSObject * obj) {
            return dynamic_cast<JSOffScreenRenderArea &>(JSOffScreenRenderArea::getJSWrapper(cx,obj));
        }

    private:
};



template <>
struct JSClassTraits<OffScreenRenderArea> : public JSClassTraitsWrapper<OffScreenRenderArea, JSOffScreenRenderArea> {
    static JSClass * Class() {
        return JSOffScreenRenderArea::Base::Class();
    }
};    


bool convertFrom(JSContext *cx, jsval theValue, OffScreenRenderArea *& theRenderArea);
    
} //namespace

#endif // _ac_jslib_JSOffScreenRenderArea_h_
