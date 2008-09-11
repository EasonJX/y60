//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _Y60_ACXPSHELL_JSSPHERE_INCLUDED_
#define _Y60_ACXPSHELL_JSSPHERE_INCLUDED_

#include "JSWrapper.h"
#include <y60/base/DataTypes.h>
#include <asl/math/Vector234.h>
#include <asl/math/numeric_functions.h>
#include <asl/math/linearAlgebra.h>
#include <asl/math/Sphere.h>
#include <asl/dom/Value.h>


namespace jslib {

typedef float SphereNumber;

class JSSphere : public JSWrapper<asl::Sphere<SphereNumber> > {
    JSSphere() {}
public:
    typedef JSWrapper<asl::Sphere<SphereNumber> > Base;

    static const char * ClassName() {
        return "Sphere";
    }
    static JSFunctionSpec * Functions();

    enum PropertyNumbers {
        PROP_center = -100,
        PROP_radius,
        PROP_hasPosition,
        PROP_hasSize,
        PROP_hasArea,
        PROP_hasVolume,
        PROP_isEmpty,
        PROP_size,
        PROP_volume,
        PROP_area
    };

    static JSPropertySpec * Properties();
    static JSConstIntPropertySpec * ConstIntProperties();
    static JSPropertySpec * StaticProperties();
    static JSFunctionSpec * StaticFunctions();


    virtual unsigned long length() const {
        return 1;
    }
    // getproperty handling
    virtual JSBool getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);
    //virtual JSBool getPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp);

    // setproperty handling
    virtual JSBool setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);
    //virtual JSBool setPropertyIndex(unsigned long theIndex, JSContext *cx, JSObject *obj, jsval id, jsval *vp);

    static JSBool
    Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

    static
    JSObject * Construct(JSContext *cx, JSSphere::NativeValuePtr theValue) {
        return Base::Construct(cx, theValue, 0);
    }
    static
    JSObject * Construct(JSContext *cx, const asl::Sphere<SphereNumber> & theSphere) {
        JSSphere::NativeValuePtr myValue(new dom::ValueWrapper<asl::Sphere<SphereNumber> >::Type(theSphere, 0));
        return Base::Construct(cx, myValue, 0);
    }
    JSSphere(JSSphere::NativeValuePtr theValue)
        : Base(theValue, 0)
    {}

    static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);
};

template <>
struct JSClassTraits<asl::Sphere<SphereNumber> >
    : public JSClassTraitsWrapper<asl::Sphere<SphereNumber> , JSSphere> {};

bool convertFrom(JSContext *cx, jsval theValue, asl::Sphere<SphereNumber>  & theSphere);

// jsval as_jsval(JSContext *cx, const asl::Sphere<SphereNumber>  & theValue);

jsval as_jsval(JSContext *cx, asl::Sphere<SphereNumber> theValue);

jsval as_jsval(JSContext *cx, dom::ValuePtr theValue, asl::Sphere<SphereNumber>  * theSphere);

}

#endif

