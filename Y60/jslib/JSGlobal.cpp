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
//   $RCSfile: JSGlobal.cpp,v $
//   $Author: danielk $
//   $Revision: 1.13 $
//   $Date: 2005/04/29 13:25:43 $
//
//
//=============================================================================

#include "JScppUtils.h"
#include "JSVector.h"
#include "JSMatrix.h"
#include "JSLine.h"
#include "JSPlane.h"
#include "JSSphere.h"
#include "JSTriangle.h"
#include "JSQuaternion.h"
#include "JSBox.h"
#include "JSFrustum.h"

#include "JSGlobal.h"

#include <asl/linearAlgebra.h>
#include <asl/intersection.h>
#include <asl/numeric_functions.h>

#include <y60/Request.h>

#define DB(x) // x

using namespace std;
using namespace asl;

// What is this doing here? (MS & UZ)
namespace asl {
    inline
    float distance(const float & a, const float & b) {
        return fabs(a - b);
    }

    inline
    double distance(const double & a, const double & b) {
        return fabs(a - b);
    }
}

namespace jslib {

/*
template <class R, class T1, class T2>
const R & doesNotExist(const T1 &, const T2 &) {
    throw DoesNotExist(JUST_FILE_LINE);
}
*/
/*
template <class R, class T1, class T2>
R doesNotExist(T1, T2 ) {
    throw DoesNotExist(JUST_FILE_LINE);
}
*/
#ifdef _SETTING_VC_TEMPLATE_BUG_WORKAROUND_

float doesNotExistS(const float &, const float &) {
    throw DoesNotExist(JUST_FILE_LINE);
}
double doesNotExistS(const double &, const double &) {
    throw DoesNotExist(JUST_FILE_LINE);
}
float doesNotExistS(const float &) {
    throw DoesNotExist(JUST_FILE_LINE);
}
double doesNotExistS(const double &) {
    throw DoesNotExist(JUST_FILE_LINE);
}

#if 1

#define DEF_DNE(RET,VEC)\
VEC doesNotExistV(const VEC & a) {\
    AC_ERROR << "acLinAlg: function with arguments type " << JValueTypeTraits<VEC>::Name() << " does not exist." << endl;\
    throw DoesNotExist(JUST_FILE_LINE);\
}\
VEC doesNotExistV(const VEC & a, const VEC &) {\
    AC_ERROR << "acLinAlg: function with arguments type " << JValueTypeTraits<VEC>::Name() << " does not exist." << endl;\
    throw DoesNotExist(JUST_FILE_LINE);\
}\
RET doesNotExistS(const VEC & a) {\
    AC_ERROR << "acLinAlg: function with arguments type " << JValueTypeTraits<VEC>::Name() << " does not exist." << endl;\
    throw DoesNotExist(JUST_FILE_LINE);\
}\
RET doesNotExistS(const VEC & a, const VEC &) {\
    AC_ERROR << "acLinAlg: function with arguments type " << JValueTypeTraits<VEC>::Name() << " does not exist." << endl;\
    throw DoesNotExist(JUST_FILE_LINE);\
}

DEF_DNE(double,asl::Vector2d);
DEF_DNE(double,asl::Vector3d);
DEF_DNE(double,asl::Vector4d);
DEF_DNE(float,asl::Vector2f);
DEF_DNE(float,asl::Vector3f);
DEF_DNE(float,asl::Vector4f);
DEF_DNE(int,asl::Vector2i);
DEF_DNE(int,asl::Vector3i);
DEF_DNE(int,asl::Vector4i);

#endif

#else
template <class R, class T1>
R doesNotExistV(T1) {
    throw DoesNotExist(JUST_FILE_LINE);
}
template <class R, class T1, class T2>
R doesNotExistV(T1, T2 ) {
    throw DoesNotExist(JUST_FILE_LINE);
}
template <class R, class T1>
R doesNotExistS(T1) {
    throw DoesNotExist(JUST_FILE_LINE);
}
template <class R, class T1, class T2>
R doesNotExistS(T1, T2 ) {
    throw DoesNotExist(JUST_FILE_LINE);
}
#endif



enum CallStatus { NOT_FOUND = 0, CALLED = 1, FAILED = 2 };

template <class RESULT_TYPE, class NATIVE_VECTORA>
static CallStatus
VectorFunction(RESULT_TYPE (*theFunction)(const NATIVE_VECTORA &),
               JSContext *cx, JSObject *myObj0, jsval *rval)
{
    // check argument for native type
    if (!JS_InstanceOf(cx,myObj0,JSVector<NATIVE_VECTORA>::Class(),0)) {
        // try to create temporary from first argument
        jsuint myArrayLength = 0;
        if (!JS_HasArrayLength(cx,myObj0,&myArrayLength)) {
            DB(AC_TRACE << "VectorFunction:: first argument must be a vector or array" << endl);
            return NOT_FOUND;
        }
        if (myArrayLength == JSVector<NATIVE_VECTORA>::SIZE) {
            myObj0 = JSVector<NATIVE_VECTORA>::Construct(cx, OBJECT_TO_JSVAL(myObj0));
            if (!myObj0) {
                return FAILED;
            }
        } else {
            return NOT_FOUND;
        }
    }
    // make the actual call
    RESULT_TYPE myResult = theFunction(
        JSClassTraits<NATIVE_VECTORA>::getNativeRef(cx,myObj0));
        //JSVector<NATIVE_VECTORA>::getNative(cx,myObj0));
    *rval = as_jsval(cx, myResult);
    return CALLED;

}

template <class RESULT_TYPE, class NATIVE_VECTORA, class NATIVE_VECTORB>
static CallStatus
VectorVectorFunction(RESULT_TYPE (*theFunction)(const NATIVE_VECTORA &,const NATIVE_VECTORB &),
                     JSContext *cx, JSObject *myObj0, JSObject *myObj1, jsval *rval)
{
    // check first argument for native type
    if (!JS_InstanceOf(cx,myObj0,JSVector<NATIVE_VECTORA>::Class(),0)) {
        // try to create temporary from first argument
        jsuint myArrayLength = 0;
        if (!JS_HasArrayLength(cx,myObj0,&myArrayLength)) {
            DB(AC_TRACE << "VectorVectorFunction:: first argument must be a vector or array" << endl);
            return NOT_FOUND;
        }
        if (myArrayLength == JSVector<NATIVE_VECTORA>::SIZE) {
            myObj0 = JSVector<NATIVE_VECTORA>::Construct(cx, OBJECT_TO_JSVAL(myObj0));
            if (!myObj0) {
                return FAILED;
            }
        } else {
            return NOT_FOUND;
        }
    }
    // check second argument for native type
    if (!JS_InstanceOf(cx,myObj1,JSVector<NATIVE_VECTORB>::Class(),0)) {
        // create temporary second argument
        myObj1 = JSVector<NATIVE_VECTORB>::Construct(cx, OBJECT_TO_JSVAL(myObj1));
        if (!myObj1) {
            return FAILED;
        }
    }
    // make the actual call
    RESULT_TYPE myResult = theFunction(
        JSClassTraits<NATIVE_VECTORA>::getNativeRef(cx,myObj0),
        JSClassTraits<NATIVE_VECTORB>::getNativeRef(cx,myObj1));
        //JSVector<NATIVE_VECTORA>::getNative(cx,myObj0),
        //JSVector<NATIVE_VECTORB>::getNative(cx,myObj1));
    *rval = as_jsval(cx, myResult);
    return CALLED;

}
template <class RESULT_TYPE, class NATIVE_NUMBERA>
static CallStatus
NumberFunction(RESULT_TYPE (*theFunction)(const NATIVE_NUMBERA &),
               JSContext *cx, jsval theValue0, jsval *rval)
{
    jsdouble myArg0 = -1;
    if (JS_ValueToNumber(cx, theValue0, &myArg0) &&
        !JSDOUBLE_IS_NaN(myArg0) )
    {
        IF_NOISY(AC_TRACE << "NumberFunction: value is a number :"<<myArg0 << endl);
        RESULT_TYPE myResult = theFunction(static_cast<NATIVE_NUMBERA>(myArg0));
        *rval = as_jsval(cx, myResult);
        return CALLED;
    }
    return NOT_FOUND;
}

template <class RESULT_TYPE, class NATIVE_NUMBERA, class NATIVE_NUMBERB>
static CallStatus
NumberNumberFunction(RESULT_TYPE (*theFunction)(const NATIVE_NUMBERA &,const NATIVE_NUMBERB &),
                     JSContext *cx, jsval theValue0, jsval theValue1, jsval *rval)
{
    jsdouble myArg0 = -1;
    jsdouble myArg1 = -1;
    if (JS_ValueToNumber(cx, theValue0, &myArg0) &&
        JS_ValueToNumber(cx, theValue1, &myArg1) &&
        !JSDOUBLE_IS_NaN(myArg0) &&
        !JSDOUBLE_IS_NaN(myArg1) )
    {
        IF_NOISY(AC_TRACE << "NumberNumberFunction: values are numbers :"<<myArg0<<" and "<< myArg1 << endl);
        RESULT_TYPE myResult = theFunction(static_cast<NATIVE_NUMBERA>(myArg0),static_cast<NATIVE_NUMBERB>(myArg1));
        *rval = as_jsval(cx, myResult);
        return CALLED;
    }
    return NOT_FOUND;
}

template <class RESULT_TYPE, class NATIVE_VECTORA, class NATIVE_NUMBERB>
static CallStatus
VectorNumberFunction(RESULT_TYPE (*theFunction)(const NATIVE_VECTORA &,NATIVE_NUMBERB),
                     JSContext *cx, JSObject *myObj0, jsval theValue1, jsval *rval)
{
    // check first argument for native type
    if (!JS_InstanceOf(cx,myObj0,JSVector<NATIVE_VECTORA>::Class(),0)) {
        // try to create temporary from first argument
        jsuint myArrayLength = 0;
        if (!JS_HasArrayLength(cx,myObj0,&myArrayLength)) {
            DB(AC_TRACE << "VectorNumberFunction:: first argument must be a vector or array" << endl);
            return NOT_FOUND;
        }
        if (myArrayLength == JSVector<NATIVE_VECTORA>::SIZE) {
            myObj0 = JSVector<NATIVE_VECTORA>::Construct(cx, OBJECT_TO_JSVAL(myObj0));
            if (!myObj0) {
                return FAILED;
            }
        } else {
            return NOT_FOUND;
        }
    }
    // check second argument for number type
    jsdouble myArg1 = -1;
    if (!JS_ValueToNumber(cx, theValue1, &myArg1) || JSDOUBLE_IS_NaN(myArg1) )
    {
        return NOT_FOUND;
    }
    // make the actual call
    RESULT_TYPE myResult = theFunction(
        JSClassTraits<NATIVE_VECTORA>::getNativeRef(cx,myObj0),
        //JSVector<NATIVE_VECTORA>::getNative(cx,myObj0),
        static_cast<NATIVE_NUMBERB>(myArg1));
    *rval = as_jsval(cx, myResult);
    return CALLED;
}

template <class RESULT_TYPE, class NATIVE_VECTORA, class NATIVE_B>
static CallStatus
VectorObjectFunction(RESULT_TYPE (*theFunction)(const NATIVE_VECTORA &,const NATIVE_B &),
                     JSContext *cx, JSObject *myObj0, JSObject *myObj1, jsval *rval)
{
    // check first argument for native type
    if (!JS_InstanceOf(cx,myObj0,JSVector<NATIVE_VECTORA>::Class(),0)) {
        // try to create temporary from first argument
        jsuint myArrayLength = 0;
        if (!JS_HasArrayLength(cx,myObj0,&myArrayLength)) {
            DB(AC_TRACE << "VectorObjectFunction:: first argument must be a vector or array" << endl);
            return NOT_FOUND;
        }
        if (myArrayLength == JSVector<NATIVE_VECTORA>::SIZE) {
            myObj0 = JSVector<NATIVE_VECTORA>::Construct(cx, OBJECT_TO_JSVAL(myObj0));
            if (!myObj0) {
                return FAILED;
            }
        } else {
            return NOT_FOUND;
        }
    }
    // check second argument for native type
    if (!JS_InstanceOf(cx,myObj1,JSClassTraits<NATIVE_B>::Class(),0)) {
        return NOT_FOUND;
    }
    // make the actual call
    RESULT_TYPE myResult = theFunction(
        //JSVector<NATIVE_VECTORA>::getNative(cx,myObj0),
        JSClassTraits<NATIVE_VECTORA>::getNativeRef(cx,myObj0),
        JSClassTraits<NATIVE_B>::getNativeRef(cx,myObj1));
    *rval = as_jsval(cx, myResult);
    return CALLED;

}
template <class RESULT_TYPE, class NATIVE_A, class NATIVE_VECTORB>
static CallStatus
ObjectVectorFunction(RESULT_TYPE (*theFunction)(const NATIVE_A &,const NATIVE_VECTORB &),
                     JSContext *cx, JSObject *myObj0, JSObject *myObj1, jsval *rval)
{
    // check first argument for native type
    if (!JS_InstanceOf(cx,myObj0,JSClassTraits<NATIVE_A>::Class(),0)) {
        return NOT_FOUND;
    }
     // check second argument for native type
    if (!JS_InstanceOf(cx,myObj1,JSVector<NATIVE_VECTORB>::Class(),0)) {
        // create temporary second argument
        myObj1 = JSVector<NATIVE_VECTORB>::Construct(cx, OBJECT_TO_JSVAL(myObj1));
        if (!myObj1) {
            return FAILED;
        }
    }
    // make the actual call
    RESULT_TYPE myResult = theFunction(
        JSClassTraits<NATIVE_A>::getNativeRef(cx,myObj0),
        JSClassTraits<NATIVE_VECTORB>::getNativeRef(cx,myObj1));
        //JSVector<NATIVE_VECTORB>::getNative(cx,myObj1));
    *rval = as_jsval(cx, myResult);
    return CALLED;

}

template <class RESULT_TYPE, class NATIVE_A>
static CallStatus
ObjectFunction(RESULT_TYPE (*theFunction)(const NATIVE_A &),
                     JSContext *cx, JSObject *myObj0, jsval *rval)
{
    // check first argument for native type
    if (!JS_InstanceOf(cx,myObj0,JSClassTraits<NATIVE_A>::Class(),0)) {
        return NOT_FOUND;
    }

    // make the actual call
    RESULT_TYPE myResult = theFunction(
        JSClassTraits<NATIVE_A>::getNativeRef(cx,myObj0));
    *rval = as_jsval(cx, myResult);
    return CALLED;

}

template <class RESULT_TYPE, class NATIVE_A, class NATIVE_B>
static CallStatus
ObjectObjectFunction(RESULT_TYPE (*theFunction)(const NATIVE_A &,const NATIVE_B &),
                     JSContext *cx, JSObject *myObj0, JSObject *myObj1, jsval *rval)
{
    // check first argument for native type
    if (!JS_InstanceOf(cx,myObj0,JSClassTraits<NATIVE_A>::Class(),0)) {
        return NOT_FOUND;
    }
    // check second argument for native type
    if (!JS_InstanceOf(cx,myObj1,JSClassTraits<NATIVE_B>::Class(),0)) {
        return FAILED;
    }
    // make the actual call
    RESULT_TYPE myResult = theFunction(
        JSClassTraits<NATIVE_A>::getNativeRef(cx,myObj0),
        JSClassTraits<NATIVE_B>::getNativeRef(cx,myObj1));
    *rval = as_jsval(cx, myResult);
    return CALLED;
}

template <
    class NATIVE_NUMBER1A,
    class NATIVE_VECTOR2A,
    class NATIVE_VECTOR3A,
    class NATIVE_VECTOR4A,
    class RESULT_TYPE1,
    class RESULT_TYPE2 = RESULT_TYPE1,
    class RESULT_TYPE3 = RESULT_TYPE2,
    class RESULT_TYPE4 = RESULT_TYPE3,
    class NATIVE_NUMBER1B = NATIVE_NUMBER1A,
    class NATIVE_VECTOR2B = NATIVE_VECTOR2A,
    class NATIVE_VECTOR3B = NATIVE_VECTOR3A,
    class NATIVE_VECTOR4B = NATIVE_VECTOR4A
>
struct Call {
    static CallStatus
        dispatch(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval,
        RESULT_TYPE1 (*theFunction1)(const NATIVE_NUMBER1A &,const NATIVE_NUMBER1B &),
        RESULT_TYPE2 (*theFunction2)(const NATIVE_VECTOR2A &,const NATIVE_VECTOR2B &),
        RESULT_TYPE3 (*theFunction3)(const NATIVE_VECTOR3A &,const NATIVE_VECTOR3B &),
        RESULT_TYPE4 (*theFunction4)(const NATIVE_VECTOR4A &,const NATIVE_VECTOR4B &),
        unsigned minArgCount = 2,
        unsigned maxArgCount = 2)
    {
        if (argc != 2) {
            if (argc < minArgCount || argc > maxArgCount) {
                AC_ERROR << "dispatch: bad number of arguments: "<<argc<<", expected 2"<<endl;
                return FAILED;
            }
            return NOT_FOUND;
        }
        try {
            CallStatus myStatus = NumberNumberFunction(theFunction1,cx,argv[0],argv[1],rval);
            if (myStatus != NOT_FOUND) return myStatus;

            JSObject * myObj0 = 0;
            JSObject * myObj1 = 0;
            if (JSVAL_IS_OBJECT(argv[0]) && JSVAL_IS_OBJECT(argv[1]) &&
                JS_ValueToObject(cx, argv[0], &myObj0) == JS_TRUE &&
                JS_ValueToObject(cx, argv[1], &myObj1) == JS_TRUE)
            {
                myStatus = VectorVectorFunction(theFunction2,cx,myObj0,myObj1,rval);
                if (myStatus != NOT_FOUND) return myStatus;

                myStatus = VectorVectorFunction(theFunction3,cx,myObj0,myObj1,rval);
                if (myStatus != NOT_FOUND) return myStatus;

                myStatus = VectorVectorFunction(theFunction4,cx,myObj0,myObj1,rval);
                if (myStatus != NOT_FOUND) return myStatus;
            }
        }
        catch (DoesNotExist &) {
            return NOT_FOUND;
        }
        catch (asl::Exception & ex) {
            AC_ERROR << "Call<>::dispatch: Unexpected exception "<< ex << endl;
            return FAILED;
        }
        catch (...) {
            AC_ERROR << "Call<>::dispatch: Unexpected unknown exception" << endl;
            return FAILED;
        }
        return NOT_FOUND;
    }
#if 1
    static CallStatus
        dispatch(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval,
        RESULT_TYPE1 (*theFunction1)(const NATIVE_NUMBER1A &),
        RESULT_TYPE2 (*theFunction2)(const NATIVE_VECTOR2A &),
        RESULT_TYPE3 (*theFunction3)(const NATIVE_VECTOR3A &),
        RESULT_TYPE4 (*theFunction4)(const NATIVE_VECTOR4A &),
        unsigned minArgCount = 1,
        unsigned maxArgCount = 1)
    {
        if (argc != 1) {
            if (argc < minArgCount || argc > maxArgCount) {
                AC_ERROR << "dispatch: bad number of arguments: "<<argc<<", expected 2"<<endl;
                return FAILED;
            }
            return NOT_FOUND;
        }
        try {
            CallStatus myStatus = NumberFunction(theFunction1,cx,argv[0],rval);
            if (myStatus != NOT_FOUND) return myStatus;

            JSObject * myObj0 = 0;
            if (JSVAL_IS_OBJECT(argv[0]) &&
                JS_ValueToObject(cx, argv[0], &myObj0) == JS_TRUE)
            {
                myStatus = VectorFunction(theFunction2,cx,myObj0,rval);
                if (myStatus != NOT_FOUND) return myStatus;

                myStatus = VectorFunction(theFunction3,cx,myObj0,rval);
                if (myStatus != NOT_FOUND) return myStatus;

                myStatus = VectorFunction(theFunction4,cx,myObj0,rval);
                if (myStatus != NOT_FOUND) return myStatus;
            }
        }
        catch (DoesNotExist &) {
            return NOT_FOUND;
        }
        catch (asl::Exception & ex) {
            AC_ERROR << "Call<>::dispatch: Unexpected exception "<< ex << endl;
            return FAILED;
        }
        catch (...) {
            AC_ERROR << "Call<>::dispatch: Unexpected unknown exception" << endl;
            return FAILED;
        }
        return NOT_FOUND;
    }
#endif
};

// help the compiler confused by almostEqual's default 3rd argument to find the correct function
template <class VECTOR>
static
bool almostEqualHelper(const VECTOR & a, const VECTOR & b) {
    return asl::almostEqual(a,b);
}
#define DEFINE_SCALAR_RESULT_FUNCTION(EXPORT_FUNCTION_NAME,FUNCTION_NAME1, FUNCTION_NAME2,FUNCTION_NAME3,FUNCTION_NAME4,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, MORE) \
    static JSBool\
    EXPORT_FUNCTION_NAME(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {\
        DOC_BEGIN(#EXPORT_FUNCTION_NAME);\
        DOC_RVAL(#RETURN_VALUE_TYPE1, DOC_TYPE_INTEGER);\
        DOC_END;\
        CallStatus myStatus = NOT_FOUND;\
        MORE;\
        myStatus =\
            Call<float,asl::Vector2f,asl::Vector3f,asl::Vector4f,RETURN_VALUE_TYPE1>::dispatch(cx,obj,argc,argv,rval,\
                &FUNCTION_NAME1,&FUNCTION_NAME2, &FUNCTION_NAME3, &FUNCTION_NAME4);\
        if (myStatus == CALLED) return JS_TRUE;\
        myStatus =\
            Call<double,asl::Vector2d,asl::Vector3d,asl::Vector4d,RETURN_VALUE_TYPE2>::dispatch(cx,obj,argc,argv,rval,\
                &FUNCTION_NAME1,&FUNCTION_NAME2, &FUNCTION_NAME3, &FUNCTION_NAME4);\
        if (myStatus == CALLED) return JS_TRUE;\
        JS_ReportError(cx,"%s: argument type mismatch, no valid type combination found", # EXPORT_FUNCTION_NAME); \
        return JS_FALSE;\
    }

#define DEFINE_SCALAR_RESULT_FUNCTION_1234_MORE(EXPORT_FUNCTION_NAME, FUNCTION_NAME_1234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, MORE) \
    DEFINE_SCALAR_RESULT_FUNCTION(EXPORT_FUNCTION_NAME,FUNCTION_NAME_1234, FUNCTION_NAME_1234, FUNCTION_NAME_1234, FUNCTION_NAME_1234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, MORE)

#define DEFINE_SCALAR_RESULT_FUNCTION_234(EXPORT_FUNCTION_NAME, FUNCTION_NAME_234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2) \
    DEFINE_SCALAR_RESULT_FUNCTION(EXPORT_FUNCTION_NAME,doesNotExistS, FUNCTION_NAME_234, FUNCTION_NAME_234, FUNCTION_NAME_234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, )

#define DEFINE_SCALAR_RESULT_FUNCTION_23(EXPORT_FUNCTION_NAME, FUNCTION_NAME_23,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2) \
    DEFINE_SCALAR_RESULT_FUNCTION(EXPORT_FUNCTION_NAME,doesNotExistS, FUNCTION_NAME_23, FUNCTION_NAME_23, doesNotExistS,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, )

#define DEFINE_VECTOR_RESULT_FUNCTION(EXPORT_FUNCTION_NAME,FUNCTION_NAME1, FUNCTION_NAME2, FUNCTION_NAME3, FUNCTION_NAME4, RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, MORE) \
    static JSBool\
    EXPORT_FUNCTION_NAME(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {\
    DOC_BEGIN(#EXPORT_FUNCTION_NAME);\
    DOC_RVAL(#RETURN_VALUE_TYPE2, DOC_TYPE_VECTOR3F);\
    DOC_END;\
        CallStatus myStatus = NOT_FOUND;\
        MORE;\
        myStatus =\
            Call<float,asl::Vector2f,asl::Vector3f,asl::Vector4f,RETURN_VALUE_TYPE1,\
                asl::Vector2f,asl::Vector3f,asl::Vector4f>::dispatch(cx,obj,argc,argv,rval,\
                    &FUNCTION_NAME1,&FUNCTION_NAME2, &FUNCTION_NAME3, &FUNCTION_NAME4);\
        if (myStatus == CALLED) return JS_TRUE;\
        myStatus =\
            Call<double,asl::Vector2d,asl::Vector3d,asl::Vector4d,RETURN_VALUE_TYPE2,\
                asl::Vector2d,asl::Vector3d,asl::Vector4d>::dispatch(cx,obj,argc,argv,rval,\
                    &FUNCTION_NAME1,&FUNCTION_NAME2, &FUNCTION_NAME3, &FUNCTION_NAME4);\
        if (myStatus == CALLED) return JS_TRUE;\
        JS_ReportError(cx,"%s: argument type mismatch, no valid type combination found", # EXPORT_FUNCTION_NAME); \
        return JS_FALSE;\
    }

#define DEFINE_VECTOR_RESULT_FUNCTION_1234_MORE(EXPORT_FUNCTION_NAME, FUNCTION_NAME_1234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, MORE_LINES) \
    DEFINE_VECTOR_RESULT_FUNCTION(EXPORT_FUNCTION_NAME,FUNCTION_NAME_1234, FUNCTION_NAME_1234, FUNCTION_NAME_1234, FUNCTION_NAME_1234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, MORE_LINES)

#define DEFINE_VECTOR_RESULT_FUNCTION_1234(EXPORT_FUNCTION_NAME, FUNCTION_NAME_1234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2) \
    DEFINE_VECTOR_RESULT_FUNCTION(EXPORT_FUNCTION_NAME,FUNCTION_NAME_1234, FUNCTION_NAME_1234, FUNCTION_NAME_1234, FUNCTION_NAME_1234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, )

#define DEFINE_VECTOR_RESULT_FUNCTION_234_MORE(EXPORT_FUNCTION_NAME, FUNCTION_NAME_234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, MORE) \
    DEFINE_VECTOR_RESULT_FUNCTION(EXPORT_FUNCTION_NAME,doesNotExistS, FUNCTION_NAME_234, FUNCTION_NAME_234, FUNCTION_NAME_234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, MORE)

#define DEFINE_VECTOR_RESULT_FUNCTION_234(EXPORT_FUNCTION_NAME, FUNCTION_NAME_234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2) \
    DEFINE_VECTOR_RESULT_FUNCTION(EXPORT_FUNCTION_NAME,doesNotExistS, FUNCTION_NAME_234, FUNCTION_NAME_234, FUNCTION_NAME_234,RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, )

#define DEFINE_VECTOR_RESULT_FUNCTION_23(EXPORT_FUNCTION_NAME,FUNCTION_NAME_23, RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2) \
    DEFINE_VECTOR_RESULT_FUNCTION(EXPORT_FUNCTION_NAME,doesNotExistS, FUNCTION_NAME_23,  FUNCTION_NAME_23,   doesNotExistV, RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, )

#define DEFINE_VECTOR_RESULT_FUNCTION_3(EXPORT_FUNCTION_NAME,FUNCTION_NAME_3, RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2) \
    DEFINE_VECTOR_RESULT_FUNCTION(EXPORT_FUNCTION_NAME,doesNotExistS, doesNotExistV,      FUNCTION_NAME_3,   doesNotExistV, RETURN_VALUE_TYPE1, RETURN_VALUE_TYPE2, )

static CallStatus
moreAlmostEqual(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    if (argc == 2 && JSVAL_IS_OBJECT(argv[0]) && JSVAL_IS_OBJECT(argv[1])) {
        CallStatus myStatus;
        {
            typedef bool (*MyFunctionPtr)(const asl::Matrix4f &, const asl::Matrix4f &);
            myStatus = ObjectObjectFunction((MyFunctionPtr)&almostEqualHelper,cx,JSVAL_TO_OBJECT(argv[0]),JSVAL_TO_OBJECT(argv[1]),rval);
            if (myStatus != NOT_FOUND) return myStatus;
        }{
            typedef bool (*MyFunctionPtr)(const asl::Quaternionf &, const asl::Quaternionf &);
            myStatus = ObjectObjectFunction((MyFunctionPtr)&almostEqualHelper,cx,JSVAL_TO_OBJECT(argv[0]),JSVAL_TO_OBJECT(argv[1]),rval);
            if (myStatus != NOT_FOUND) return myStatus;
        }
    }
    return NOT_FOUND;
}
#define MORE_ALMOST_EQUAL \
    myStatus = moreAlmostEqual(cx,obj,argc,argv,rval);\
    if (myStatus == CALLED) return JS_TRUE;

DEFINE_SCALAR_RESULT_FUNCTION_1234_MORE(almostEqual,almostEqualHelper, bool, bool, MORE_ALMOST_EQUAL );
DEFINE_SCALAR_RESULT_FUNCTION_234(dotProduct, asl::dot,float, double);
DEFINE_VECTOR_RESULT_FUNCTION_234(sum, asl::sum, float, double);
DEFINE_VECTOR_RESULT_FUNCTION_234(difference, asl::difference,float, double);

template <class VECTOR, class NUMBER>
struct Helper {
    static CallStatus
    VectorNumberProduct(JSContext *cx, JSObject *obj0, jsval val1, jsval *rval) {
        typedef VECTOR (*MyFunctionPtr)(const VECTOR &, NUMBER);
        return VectorNumberFunction((MyFunctionPtr)&asl::product,cx,obj0,val1,rval);
    }
};

// XXX: MS & DS: order is important here! ... be careful!
static CallStatus
productHelper(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    CallStatus myStatus = NOT_FOUND;
    if (argc == 2 ) {
        if (JSVAL_IS_OBJECT(argv[0])) {
            JSObject * myObj0 = JSVAL_TO_OBJECT(argv[0]);
            if (JSVAL_IS_OBJECT(argv[1])) {

                JSObject * myObj1 = JSVAL_TO_OBJECT(argv[1]);

                {
                    typedef asl::Vector4f (*MyFunctionPtr)(const asl::Vector4f &, const asl::Matrix4f &);
                    myStatus = VectorObjectFunction((MyFunctionPtr)&asl::product,cx,myObj0,myObj1,rval);
                    if (myStatus != NOT_FOUND) return myStatus;
                }{
                    typedef asl::Point3f (*MyFunctionPtr)(const asl::Point3f &, const asl::Matrix4f &);
                    myStatus = VectorObjectFunction((MyFunctionPtr)&asl::product,cx,myObj0,myObj1,rval);
                    if (myStatus != NOT_FOUND) return myStatus;
                }{
                    typedef asl::Quaternionf (*MyFunctionPtr)(const asl::Quaternionf &, const asl::Quaternionf &);
                    myStatus = ObjectObjectFunction((MyFunctionPtr)&asl::product,cx,myObj0,myObj1,rval);
                    if (myStatus != NOT_FOUND) return myStatus;
                }{
                    typedef asl::Vector4f (*MyFunctionPtr)(const asl::Vector4f &, const asl::Quaternionf &);
                    myStatus = VectorObjectFunction((MyFunctionPtr)&asl::product,cx,myObj0,myObj1,rval);
                    if (myStatus != NOT_FOUND) return myStatus;
                }{
                    typedef asl::Point3f (*MyFunctionPtr)(const asl::Point3f &, const asl::Quaternionf &);
                    myStatus = VectorObjectFunction((MyFunctionPtr)&asl::product,cx,myObj0,myObj1,rval);
                    if (myStatus != NOT_FOUND) return myStatus;
                }{
                    typedef asl::Box3f (*MyFunctionPtr)(const asl::Box3f &, const asl::Matrix4f &);
                    myStatus = ObjectObjectFunction((MyFunctionPtr)&asl::product,cx,myObj0,myObj1,rval);
                    if (myStatus != NOT_FOUND) return myStatus;
                }{
                    typedef asl::Planef (*MyFunctionPtr)(const asl::Planef &, const asl::Matrix4f &);
                    myStatus = ObjectObjectFunction((MyFunctionPtr)&asl::product,cx,myObj0,myObj1,rval);
                    if (myStatus != NOT_FOUND) return myStatus;
                }
            } else {
                // vector/number
                myStatus = Helper<asl::Vector2f, float>::VectorNumberProduct(cx,myObj0, argv[1], rval);
                if (myStatus != NOT_FOUND) return myStatus;

                myStatus = Helper<asl::Vector3f, float>::VectorNumberProduct(cx,myObj0, argv[1], rval);
                if (myStatus != NOT_FOUND) return myStatus;

                myStatus = Helper<asl::Vector4f, float>::VectorNumberProduct(cx,myObj0, argv[1], rval);
                if (myStatus != NOT_FOUND) return myStatus;

                myStatus = Helper<asl::Point2f, float>::VectorNumberProduct(cx,myObj0, argv[1], rval);
                if (myStatus != NOT_FOUND) return myStatus;

                myStatus = Helper<asl::Point3f, float>::VectorNumberProduct(cx,myObj0, argv[1], rval);
                if (myStatus != NOT_FOUND) return myStatus;

                myStatus = Helper<asl::Point4f, float>::VectorNumberProduct(cx,myObj0, argv[1], rval);
                if (myStatus != NOT_FOUND) return myStatus;
            }
        }
        return myStatus;
    }
    return NOT_FOUND;
}
#define MORE_PRODUCT \
    myStatus = productHelper(cx,obj,argc,argv,rval);\
    if (myStatus == CALLED) return JS_TRUE;

static CallStatus
distanceHelper(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    if (argc == 2 && JSVAL_IS_OBJECT(argv[0]) && JSVAL_IS_OBJECT(argv[1])) {

        JSObject * myObj0 = JSVAL_TO_OBJECT(argv[0]);
        JSObject * myObj1 = JSVAL_TO_OBJECT(argv[1]);
        CallStatus myStatus;

        {
            typedef float (*MyFunctionPtr)(const asl::Point3f &, const asl::Line<float> &);
            myStatus = VectorObjectFunction((MyFunctionPtr)&asl::distance,cx,myObj0,myObj1,rval);
            if (myStatus != NOT_FOUND) return myStatus;
        }{
            typedef float (*MyFunctionPtr)(const asl::Line<float> &, const asl::Point3f &);
            myStatus = ObjectVectorFunction((MyFunctionPtr)&asl::distance,cx,myObj0,myObj1,rval);
            if (myStatus != NOT_FOUND) return myStatus;
        }{
            typedef float (*MyFunctionPtr)(const asl::Point3f &, const asl::Plane<float> &);
            myStatus = VectorObjectFunction((MyFunctionPtr)&asl::distance,cx,myObj0,myObj1,rval);
            if (myStatus != NOT_FOUND) return myStatus;
        }{
            typedef float (*MyFunctionPtr)(const asl::Plane<float> &, const asl::Point3f &);
            myStatus = ObjectVectorFunction((MyFunctionPtr)&asl::distance,cx,myObj0,myObj1,rval);
            if (myStatus != NOT_FOUND) return myStatus;
        }{
            typedef float (*MyFunctionPtr)(const asl::Point3f &, const asl::LineSegment<float> &);
            myStatus = VectorObjectFunction((MyFunctionPtr)&asl::distance,cx,myObj0,myObj1,rval);
            if (myStatus != NOT_FOUND) return myStatus;
        }{
            typedef float (*MyFunctionPtr)(const asl::LineSegment<float> &, const asl::Point3f &);
            myStatus = ObjectVectorFunction((MyFunctionPtr)&asl::distance,cx,myObj0,myObj1,rval);
            if (myStatus != NOT_FOUND) return myStatus;
        }
        return myStatus;
    }
    return NOT_FOUND;
}

DEFINE_VECTOR_RESULT_FUNCTION_234_MORE(product, asl::product,float, double, MORE_PRODUCT);
DEFINE_VECTOR_RESULT_FUNCTION_234(quotient, asl::quotient,float, double);
DEFINE_VECTOR_RESULT_FUNCTION_3(cross, asl::cross, float, double);

DEFINE_SCALAR_RESULT_FUNCTION_234(magnitude, asl::length, float, double);
DEFINE_VECTOR_RESULT_FUNCTION_234(normalized, asl::normalized,float, double);
DEFINE_VECTOR_RESULT_FUNCTION_3(normal, asl::normal,float, double);
DEFINE_VECTOR_RESULT_FUNCTION_3(orthonormal, asl::orthonormal,float, double);
DEFINE_SCALAR_RESULT_FUNCTION_23(cosAngle, asl::cosAngle,float, double);

#define MORE_DISTANCE \
    myStatus = distanceHelper(cx,obj,argc,argv,rval);\
    if (myStatus == CALLED) return JS_TRUE;

DEFINE_SCALAR_RESULT_FUNCTION_1234_MORE(distance, asl::distance, float, double, MORE_DISTANCE);

//DEFINE_SCALAR_RESULT_FUNCTION_234(distance, asl::distance, float, double);


template <class NATIVE_A, class NATIVE_B, class RESULT_TYPE>
static CallStatus
ObjectObjectResultFunction(bool (*theFunction)(const NATIVE_A &,const NATIVE_B &, RESULT_TYPE &),
                           JSContext *cx, JSObject *myObj0, JSObject *myObj1, jsval *rval)
{
    // check first argument for native type
    if (!JS_InstanceOf(cx,myObj0,JSClassTraits<NATIVE_A>::Class(),0)) {
        return NOT_FOUND;
    }
    // check second argument for native type
    if (!JS_InstanceOf(cx,myObj1,JSClassTraits<NATIVE_B>::Class(),0)) {
        return NOT_FOUND;
    }
    // make the actual call
    RESULT_TYPE myResult;
    bool mySuccess = theFunction(
        JSClassTraits<NATIVE_A>::getNativeRef(cx,myObj0),
        JSClassTraits<NATIVE_B>::getNativeRef(cx,myObj1),
        myResult);
    if (mySuccess) {
        *rval = as_jsval(cx, myResult);
    }
    return CALLED;
}

template <class NATIVE_A, class NATIVE_B, class RESULT_TYPE>
static CallStatus
ObjectObjectDualResultFunction(bool (*theFunction)(const NATIVE_A &,const NATIVE_B &, RESULT_TYPE &, RESULT_TYPE &),
                           JSContext *cx, JSObject *myObj0, JSObject *myObj1, jsval *rval)
{
    // check first argument for native type
    if (!JS_InstanceOf(cx,myObj0,JSClassTraits<NATIVE_A>::Class(),0)) {
        return NOT_FOUND;
    }
    // check second argument for native type
    if (!JS_InstanceOf(cx,myObj1,JSClassTraits<NATIVE_B>::Class(),0)) {
        return NOT_FOUND;
    }
    // make the actual call
    vector<RESULT_TYPE> myResult(2);
    bool mySuccess = theFunction(
        JSClassTraits<NATIVE_A>::getNativeRef(cx,myObj0),
        JSClassTraits<NATIVE_B>::getNativeRef(cx,myObj1),
        myResult[0], myResult[1]);
    if (mySuccess) {
        *rval = as_jsval(cx, myResult);
    }
    return CALLED;
}
static JSBool
intersectionDispatcher(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Finds the intersections between two objects.");
    DOC_PARAM("theLine", "", DOC_TYPE_LINE);               DOC_PARAM("thePlane", "", DOC_TYPE_PLANE);             DOC_RESET;
    DOC_PARAM("thePlane", "", DOC_TYPE_PLANE);             DOC_PARAM("theLine", "", DOC_TYPE_LINE);               DOC_RESET;
    DOC_PARAM("theLine", "", DOC_TYPE_LINE);               DOC_PARAM("theTriangle", "", DOC_TYPE_TRIANGLE);       DOC_RESET;
    DOC_PARAM("theTriangle", "", DOC_TYPE_TRIANGLE);       DOC_PARAM("theLine", "", DOC_TYPE_LINE);               DOC_RESET;
    DOC_PARAM("theSphere", "", DOC_TYPE_SPHERE);           DOC_PARAM("theLine", "", DOC_TYPE_LINE);               DOC_RESET;
    DOC_PARAM("theRay", "", DOC_TYPE_RAY);                 DOC_PARAM("thePlane", "", DOC_TYPE_PLANE);             DOC_RESET;
    DOC_PARAM("thePlane", "", DOC_TYPE_PLANE);             DOC_PARAM("theRay", "", DOC_TYPE_RAY);                 DOC_RESET;
    DOC_PARAM("theRay", "", DOC_TYPE_RAY);                 DOC_PARAM("theTriangle", "", DOC_TYPE_TRIANGLE);       DOC_RESET;
    DOC_PARAM("theTriangle", "", DOC_TYPE_TRIANGLE);       DOC_PARAM("theRay", "", DOC_TYPE_RAY);                 DOC_RESET;
    DOC_PARAM("theLineSegment", "", DOC_TYPE_LINESEGMENT); DOC_PARAM("thePlane", "", DOC_TYPE_PLANE);             DOC_RESET;
    DOC_PARAM("thePlane", "", DOC_TYPE_PLANE);             DOC_PARAM("theLineSegment", "", DOC_TYPE_LINESEGMENT); DOC_RESET;
    DOC_PARAM("theLineSegment", "", DOC_TYPE_LINESEGMENT); DOC_PARAM("theTriangle", "", DOC_TYPE_TRIANGLE);       DOC_RESET;
    DOC_PARAM("theTriangle", "", DOC_TYPE_TRIANGLE);       DOC_PARAM("theLineSegment", "", DOC_TYPE_LINESEGMENT); DOC_RESET;
    DOC_PARAM("theBox", "", DOC_TYPE_BOX3F);               DOC_PARAM("theFrustum", "", DOC_TYPE_FRUSTUM);       DOC_RESET;
    DOC_RVAL("Returns an array of intersection structures.", DOC_TYPE_ARRAY);
    DOC_END;
    if (argc == 2) {
        JSObject * myObj0 = 0;
        JSObject * myObj1 = 0;
        if (!JSVAL_IS_OBJECT(argv[0])  || !JSVAL_IS_OBJECT(argv[1])) {
             JS_ReportError(cx,"intersectionDispatcher: passed 'undefined' as argument");
             return JS_FALSE;
        }

        if (JS_ValueToObject(cx, argv[0], &myObj0) == JS_TRUE &&
            JS_ValueToObject(cx, argv[1], &myObj1) == JS_TRUE)
        {
            CallStatus myStatus = NOT_FOUND;
            typedef bool (*LinePlaneIntersection)(const Line<LineNumber> &,const Plane<PlaneNumber> &, Point3<PlaneNumber> &);
            myStatus = ObjectObjectResultFunction((LinePlaneIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            typedef bool (*PlaneLineIntersection)(const Plane<PlaneNumber> &, const Line<LineNumber> &,Point3<PlaneNumber> &);
            myStatus = ObjectObjectResultFunction((PlaneLineIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            typedef bool (*LineTriangleIntersection)(const Line<LineNumber> &,const Triangle<TriangleNumber> &, Point3<TriangleNumber> &);
            myStatus = ObjectObjectResultFunction((LineTriangleIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            typedef bool (*TriangleLineIntersection)(const Triangle<TriangleNumber> &, const Line<LineNumber> &,Point3<TriangleNumber> &);
            myStatus = ObjectObjectResultFunction((TriangleLineIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            typedef bool (*SphereLineIntersection)(const Sphere<LineNumber> &,const Line<SphereNumber> &, Point3<SphereNumber> &, Point3<SphereNumber> &);
            myStatus = ObjectObjectDualResultFunction((SphereLineIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            // ray
            typedef bool (*RayPlaneIntersection)(const Ray<RayNumber> &,const Plane<PlaneNumber> &, Point3<PlaneNumber> &);
            myStatus = ObjectObjectResultFunction((RayPlaneIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            typedef bool (*PlaneRayIntersection)(const Plane<PlaneNumber> &, const Ray<RayNumber> &,Point3<PlaneNumber> &);
            myStatus = ObjectObjectResultFunction((PlaneRayIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            typedef bool (*RayTriangleIntersection)(const Ray<RayNumber> &,const Triangle<TriangleNumber> &, Point3<TriangleNumber> &);
            myStatus = ObjectObjectResultFunction((RayTriangleIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            typedef bool (*TriangleRayIntersection)(const Triangle<TriangleNumber> &, const Ray<RayNumber> &,Point3<TriangleNumber> &);
            myStatus = ObjectObjectResultFunction((TriangleRayIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            // LineSegment
            typedef bool (*LineSegmentPlaneIntersection)(const LineSegment<LineSegmentNumber> &,const Plane<PlaneNumber> &, Point3<PlaneNumber> &);
            myStatus = ObjectObjectResultFunction((LineSegmentPlaneIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            typedef bool (*PlaneLineSegmentIntersection)(const Plane<PlaneNumber> &, const LineSegment<LineSegmentNumber> &,Point3<PlaneNumber> &);
            myStatus = ObjectObjectResultFunction((PlaneLineSegmentIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            typedef bool (*LineSegmentTriangleIntersection)(const LineSegment<LineSegmentNumber> &,const Triangle<TriangleNumber> &, Point3<TriangleNumber> &);
            myStatus = ObjectObjectResultFunction((LineSegmentTriangleIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            typedef bool (*TriangleLineSegmentIntersection)(const Triangle<TriangleNumber> &, const LineSegment<LineSegmentNumber> &,Point3<TriangleNumber> &);
            myStatus = ObjectObjectResultFunction((TriangleLineSegmentIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            // Box / Frustum
            typedef bool (*BoxFrustumIntersection)(const Box3<TriangleNumber>&, const Frustum &);
            myStatus = ObjectObjectFunction((BoxFrustumIntersection)&intersection, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;

            JS_ReportError(cx,"intersectionDispatcher: no 'intersection' function found for this two argument types");
        } else {
            JS_ReportError(cx,"intersectionDispatcher: both arguments for 'intersection' arguments must be objects");
        }
        return JS_FALSE;
    }
    JS_ReportError(cx,"intersectionDispatcher: bad number of arguments for 'intersection'");
    return JS_FALSE;
}
static JSBool
projectionDispatcher(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Projects an geometric object onto another object");
    DOC_PARAM("theVector", "", DOC_TYPE_VECTOR3F);
    DOC_PARAM("thePlane", "", DOC_TYPE_PLANE);
    DOC_RESET;
    DOC_PARAM("theVector", "", DOC_TYPE_VECTOR3F);
    DOC_PARAM("theVector", "", DOC_TYPE_VECTOR3F);
    DOC_RVAL("The projected vector", DOC_TYPE_VECTOR3F);

    DOC_END;
    if (argc == 2) {
        JSObject * myObj0 = 0;
        JSObject * myObj1 = 0;
        if (!JSVAL_IS_OBJECT(argv[0])  || !JSVAL_IS_OBJECT(argv[1])) {
             JS_ReportError(cx,"projectionDispatcher: passed 'undefined' as argument");
             return JS_FALSE;
        }
        if (JS_ValueToObject(cx, argv[0], &myObj0) == JS_TRUE &&
            JS_ValueToObject(cx, argv[1], &myObj1) == JS_TRUE)
        {
            CallStatus myStatus = NOT_FOUND;
            {
                typedef Vector3<PlaneNumber> (*VectorPlaneProjection)(const Vector3<PlaneNumber> &, const Plane<PlaneNumber> &);
                myStatus = VectorObjectFunction((VectorPlaneProjection)&projection, cx, myObj0, myObj1, rval);
                if (myStatus != NOT_FOUND) return JS_TRUE;
            }
            {
                typedef Vector3<float> (*VectorVectorProjection)(const Vector3<float> &, const Vector3<float> &);
                myStatus = VectorVectorFunction((VectorVectorProjection)&projection, cx, myObj0, myObj1, rval);
                if (myStatus != NOT_FOUND) return JS_TRUE;
            }
            JS_ReportError(cx,"projectionDispatcher: no 'projection' function found for this two argument types");
        } else {
            JS_ReportError(cx,"projectionDispatcher: both arguments for 'projection' arguments must be objects");
        }
        return JS_FALSE;
    }
    JS_ReportError(cx,"projectionDispatcher: bad number of arguments for 'projection'");
    return JS_FALSE;
}

static JSBool
nearestDispatcher(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Finds the nearest point on a geometric object to a given point");
    DOC_PARAM("thePoint", "", DOC_TYPE_POINT3F);   DOC_PARAM("thePlane", "", DOC_TYPE_PLANE);   DOC_RESET;
    DOC_PARAM("thePlane", "", DOC_TYPE_PLANE);     DOC_PARAM("thePoint", "", DOC_TYPE_POINT3F); DOC_RESET;
    DOC_PARAM("thePoint", "", DOC_TYPE_POINT3F);   DOC_PARAM("theLine", "", DOC_TYPE_LINE);     DOC_RESET;
    DOC_PARAM("theLine", "", DOC_TYPE_LINE);       DOC_PARAM("thePoint", "", DOC_TYPE_POINT3F); DOC_RESET;
    DOC_PARAM("thePoint", "", DOC_TYPE_POINT3F);   DOC_PARAM("theSphere", "", DOC_TYPE_SPHERE); DOC_RESET;
    DOC_PARAM("theSphere", "", DOC_TYPE_SPHERE);   DOC_PARAM("thePoint", "", DOC_TYPE_POINT3F); DOC_RESET;
    DOC_RVAL("The nearest point on the object", DOC_TYPE_POINT3F);
    DOC_END;
    if (argc == 2) {
        JSObject * myObj0 = 0;
        JSObject * myObj1 = 0;
        if (!JSVAL_IS_OBJECT(argv[0])  || !JSVAL_IS_OBJECT(argv[1])) {
             JS_ReportError(cx,"nearestDispatcher: passed 'undefined' as argument");
             return JS_FALSE;
        }
        if (JS_ValueToObject(cx, argv[0], &myObj0) == JS_TRUE &&
            JS_ValueToObject(cx, argv[1], &myObj1) == JS_TRUE)
        {
            CallStatus myStatus = NOT_FOUND;
            {
            typedef Point3<PlaneNumber> (*NearestPointPlane)(const Point3<PlaneNumber> &, const Plane<PlaneNumber> &);
            myStatus = VectorObjectFunction((NearestPointPlane)&nearest, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;
            }{
            typedef Point3<PlaneNumber> (*NearestPlanePoint)(const Plane<PlaneNumber> &, const Point3<PlaneNumber> &);
            myStatus = ObjectVectorFunction((NearestPlanePoint)&nearest, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;
            }{ //--
            typedef Point3<LineNumber> (*NearestPointLine)(const Point3<LineNumber> &, const Line<LineNumber> &);
            myStatus = VectorObjectFunction((NearestPointLine)&nearest, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;
            }{
            typedef Point3<LineNumber> (*NearestLinePoint)(const Line<LineNumber> &, const Point3<LineNumber> &);
            myStatus = ObjectVectorFunction((NearestLinePoint)&nearest, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;
            }{ //--
            typedef Point3<LineNumber> (*NearestPointSphere)(const Point3<LineNumber> &, const Sphere<LineNumber> &);
            myStatus = VectorObjectFunction((NearestPointSphere)&nearest, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;
            }{
            typedef Point3<LineNumber> (*NearestSpherePoint)(const Sphere<LineNumber> &, const Point3<LineNumber> &);
            myStatus = ObjectVectorFunction((NearestSpherePoint)&nearest, cx, myObj0, myObj1, rval);
            if (myStatus != NOT_FOUND) return JS_TRUE;
            } //--
            JS_ReportError(cx,"nearestDispatcher: no 'nearest' function found for this two argument types");
        } else {
            JS_ReportError(cx,"nearestDispatcher: both arguments for 'nearest' arguments must be objects");
        }
        return JS_FALSE;
    }
    JS_ReportError(cx,"nearestDispatcher: bad number of arguments for 'nearest'");
    return JS_FALSE;
}

static JSBool
transformedNormalDispatcher(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Transforms a normal with a given matrix");
    DOC_PARAM("theNormal", "", DOC_TYPE_VECTOR3F);
    DOC_PARAM("theMatrix", "", DOC_TYPE_MATRIX4F);
    DOC_RVAL("The transformed normal", DOC_TYPE_VECTOR3F);
    DOC_END;
    if (argc == 2) {
        JSObject * myObj0 = 0;
        JSObject * myObj1 = 0;
        if (!JSVAL_IS_OBJECT(argv[0])  || !JSVAL_IS_OBJECT(argv[1])) {
             JS_ReportError(cx,"transformedNormalDispatcher: passed 'undefined' as argument");
             return JS_FALSE;
        }
        if (JS_ValueToObject(cx, argv[0], &myObj0) == JS_TRUE &&
            JS_ValueToObject(cx, argv[1], &myObj1) == JS_TRUE)
        {
            CallStatus myStatus = NOT_FOUND;
            {
                typedef Vector3<float> (*VectorMatrixTransformation)(const Vector3<float> &, const Matrix4<float> &);
                myStatus = VectorObjectFunction((VectorMatrixTransformation)&transformedNormal, cx, myObj0, myObj1, rval);
                if (myStatus != NOT_FOUND) return JS_TRUE;
            }
            JS_ReportError(cx,"transformedNormalDispatcher: no 'transformedNormal' function found for this two argument types");
        } else {
            JS_ReportError(cx,"transformedNormalDispatcher: both arguments for 'transformedNormal' arguments must be objects");
        }
        return JS_FALSE;
    }
    JS_ReportError(cx,"transformedNormalDispatcher: bad number of arguments for 'transformedNormal'");
    return JS_FALSE;
}

static JSBool
inverseDispatcher(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Calculates the inverse matrix");
    DOC_PARAM("theMatrix to invert", "", DOC_TYPE_MATRIX4F);
    DOC_RVAL("The inverted matrix", DOC_TYPE_MATRIX4F);
    DOC_END;
    if (argc == 2) {
        JSObject * myObj0 = 0;
        if (!JSVAL_IS_OBJECT(argv[0])) {
             JS_ReportError(cx,"inverseDispatcher: passed 'undefined' as argument");
             return JS_FALSE;
        }
        if (JS_ValueToObject(cx, argv[0], &myObj0) == JS_TRUE)
        {
            CallStatus myStatus = NOT_FOUND;
            {
                typedef Matrix4<float> (*MatrixInversion)(const Matrix4<float> &);
                myStatus = ObjectFunction((MatrixInversion)&inverse, cx, myObj0, rval);
                if (myStatus != NOT_FOUND) return JS_TRUE;
            }
            JS_ReportError(cx,"inverseDispatcher: no 'inverse' function found for this two argument types");
        } else {
            JS_ReportError(cx,"inverseDispatcher: both arguments for 'inverse' arguments must be objects");
        }
        return JS_FALSE;
    }
    JS_ReportError(cx,"inverseDispatcher: bad number of arguments for 'inverse'");
    return JS_FALSE;
}

static JSBool
signedDistance(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Calculates the signed distance between a point and a plane");
    DOC_PARAM("thePoint", "", DOC_TYPE_POINT3F);
    DOC_PARAM("thePlane", "", DOC_TYPE_PLANE);
    DOC_RVAL("The signed distance between point and plane", DOC_TYPE_MATRIX4F);
    DOC_END;
    if (argc == 2) {
        Point3f myPoint;
        convertFrom(cx, argv[0], myPoint);

        Planef myPlane;
        convertFrom(cx, argv[1], myPlane);
        
        *rval = as_jsval(cx, asl::signedDistance(myPoint, myPlane));
        return JS_TRUE;
    }
    JS_ReportError(cx,"inverseDispatcher: bad number of arguments for 'signedDistance'");
    return JS_FALSE;

}

JSFunctionSpec *
Global::Functions() {
    static JSFunctionSpec myFunctions[] = {
        /* name         native          nargs    */
        {"almostEqual", almostEqual,       2},
        {"dot",         dotProduct,        2},
        {"sum",         sum,               2},
        {"difference",  difference,        2},
        {"product",     product,           2},
        {"quotient",    quotient,          2},
        {"cross",       cross   ,          2},
        {"magnitude",    magnitude,        1},
        {"normalized",   normalized,       1},
        {"normal",       normal,           2},
        {"orthonormal",  orthonormal,      1},
        {"cosAngle",     cosAngle   ,      2},
        {"distance",     distance,   2},
        {"signedDistance", signedDistance, 2},
        {"intersection", intersectionDispatcher,  2},
        {"projection",  projectionDispatcher,  2},
        {"nearest",     nearestDispatcher,     2},
        {"transformedNormal", transformedNormalDispatcher,     2},

        {0}
    };
    return myFunctions;
}

}
