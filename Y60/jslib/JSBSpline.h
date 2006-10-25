//=============================================================================
// Copyright (C) 2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _Y60_JSBSPLINE_H_
#define _Y60_JSBSPLINE_H_

#include <Y60/JSWrapper.h>

#include <asl/Vector234.h>
#include <asl/BSpline.h>
#include <dom/Value.h>

namespace jslib {

typedef float BSplineNumber;

class JSBSpline : public JSWrapper<asl::BSpline<BSplineNumber>, asl::Ptr<asl::BSpline<BSplineNumber> >, StaticAccessProtocol>
{
    JSBSpline() {}

public:
    typedef asl::BSpline<BSplineNumber> NATIVE;
    typedef asl::Ptr<NATIVE> OWNERPTR;
    typedef JSWrapper<NATIVE, OWNERPTR, StaticAccessProtocol> Base;

    static const char * ClassName() {
        return "BSpline";
    };

    enum PropertyNumbers {
        PROP_start = -100,
        PROP_end,        
        PROP_starthandle,
        PROP_endhandle,
        PROP_isLineSegment
    };

    static JSFunctionSpec * Functions();
    static JSPropertySpec * Properties();
    static JSConstIntPropertySpec * ConstIntProperties();
    static JSPropertySpec * StaticProperties();
    static JSFunctionSpec * StaticFunctions();

    virtual unsigned long length() const {
        return 1;
    }

    // getproperty handling
    virtual JSBool getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);

    // setproperty handling
    virtual JSBool setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp);

    static JSBool
    Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

    static
    JSObject * Construct(JSContext *cx, OWNERPTR theOwner, NATIVE * theNative) {
        return Base::Construct(cx, theOwner, theNative);
    }

    JSBSpline(OWNERPTR theOwner, NATIVE * theNative) : Base(theOwner, theNative) {
    }

    static JSObject * initClass(JSContext *cx, JSObject * theGlobalObject);
};

bool convertFrom(JSContext * cx, jsval theValue, JSBSpline::NATIVE *& theBSpline);
bool convertFrom(JSContext * cx, jsval theValue, JSBSpline::NATIVE & theBSpline);

jsval as_jsval(JSContext * cx, JSBSpline::OWNERPTR theOwner);
jsval as_jsval(JSContext * cx, JSBSpline::OWNERPTR theOwner, JSBSpline::NATIVE * theBSpline);

} // namespace

#endif
