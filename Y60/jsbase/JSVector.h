//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _Y60_ACXPSHELL_JSVECTOR_INCLUDED_
#define _Y60_ACXPSHELL_JSVECTOR_INCLUDED_

#include "JScppUtils.h"
#include "JSWrapper.h"

#include <asl/base/settings.h>
#include <asl/base/Singleton.h>
#include <y60/base/DataTypes.h>
#include <asl/math/Vector234.h>

namespace jslib {

template<class NATIVE_VECTOR> struct JSVector;

bool convertFrom(JSContext *cx, jsval theValue, asl::Vector2i & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Vector3i & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Vector4i & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Vector2f & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Vector3f & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Vector4f & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Vector2d & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Vector3d & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Vector4d & theVector);

bool convertFrom(JSContext *cx, jsval theValue, asl::Point2i & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Point3i & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Point4i & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Point2f & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Point3f & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Point4f & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Point2d & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Point3d & theVector);
bool convertFrom(JSContext *cx, jsval theValue, asl::Point4d & theVector);

template <template<class> class NATIVE_VECTOR, class NUMBER>
jsval as_jsval(JSContext *cx, const NATIVE_VECTOR<NUMBER> & theValue) {
    JSObject * myReturnObject = JSVector<NATIVE_VECTOR<NUMBER> >::Construct(cx, theValue);
    return OBJECT_TO_JSVAL(myReturnObject);
}

template <template<class> class NATIVE_VECTOR, class NUMBER>
jsval as_jsval(JSContext *cx, dom::ValuePtr theValue, NATIVE_VECTOR<NUMBER> *) {
    JSObject * myReturnObject = JSVector<NATIVE_VECTOR<NUMBER> >::Construct(cx, theValue);
    return OBJECT_TO_JSVAL(myReturnObject);
}

template <template <class> class NATIVE_VECTOR, class NUMBER>
jsval as_jsval(JSContext *cx,
               asl::Ptr<dom::VectorValue<NATIVE_VECTOR<NUMBER> >,dom::ThreadingModel> theValue)
{
    JSObject * myReturnObject = JSVector<NATIVE_VECTOR<NUMBER> >::Construct(cx, theValue);
    return OBJECT_TO_JSVAL(myReturnObject);
}

template <>
struct JValueTypeTraits<asl::Vector2f> {
    typedef asl::Vector2f self_type;
    typedef float elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef self_type vector_type;
    static std::string Postfix() {
        return "f";
    }
    static std::string Name() {
        return "Vector2f";
    }
    enum {SIZE = 2};
};
template <>
struct JValueTypeTraits<asl::Vector3f> {
    typedef asl::Vector3f self_type;
    typedef float elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef self_type vector_type;
    static std::string Postfix() {
        return "f";
    }
    static std::string Name() {
        return "Vector3f";
    }
    enum {SIZE = 3};
};
template <>
struct JValueTypeTraits<asl::Vector4f> {
    typedef asl::Vector4f self_type;
    typedef float elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef self_type vector_type;
    static std::string Postfix() {
        return "f";
    }
    static std::string Name() {
        return "Vector4f";
    }
    enum {SIZE = 4};
};
template <>
struct JValueTypeTraits<asl::Vector2d> {
    typedef asl::Vector2d self_type;
    typedef double elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef self_type vector_type;
    static std::string Postfix() {
        return "d";
    }
    static std::string Name() {
        return "Vector2d";
    }
    enum {SIZE = 2};
};
template <>
struct JValueTypeTraits<asl::Vector3d> {
    typedef asl::Vector3d self_type;
    typedef double elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef self_type vector_type;
    static std::string Postfix() {
        return "d";
    }
    static std::string Name() {
        return "Vector3d";
    }
    enum {SIZE = 3};
};
template <>
struct JValueTypeTraits<asl::Vector4d> {
    typedef asl::Vector4d self_type;
    typedef double elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef self_type vector_type;
    static std::string Postfix() {
        return "d";
    }
    static std::string Name() {
        return "Vector4d";
    }
    enum {SIZE = 4};
};

template <>
struct JValueTypeTraits<asl::Vector2i> {
    typedef asl::Vector2i self_type;
    typedef int elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef self_type vector_type;
    static std::string Postfix() {
        return "i";
    }
    static std::string Name() {
        return "Vector2i";
    }
    enum {SIZE = 2};
};

template <>
struct JValueTypeTraits<asl::Vector3i> {
    typedef asl::Vector3i self_type;
    typedef int elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef self_type vector_type;
    static std::string Postfix() {
        return "i";
    }
    static std::string Name() {
        return "Vector3i";
    }
    enum {SIZE = 3};
};
template <>
struct JValueTypeTraits<asl::Vector4i> {
    typedef asl::Vector4i self_type;
    typedef int elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef self_type vector_type;
    static std::string Postfix() {
        return "i";
    }
    static std::string Name() {
        return "Vector4i";
    }
    enum {SIZE = 4};
};

template <>
struct JValueTypeTraits<asl::Point2d> {
    typedef asl::Point2d self_type;
    typedef double elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef asl::Vector2d vector_type;
    static std::string Postfix() {
        return "d";
    }
    static std::string Name() {
        return "Point2d";
    }
    enum {SIZE = 2};
};

template <>
struct JValueTypeTraits<asl::Point3d> {
    typedef asl::Point3d self_type;
    typedef double elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef asl::Vector3d vector_type;
    static std::string Postfix() {
        return "d";
    }
    static std::string Name() {
        return "Point3d";
    }
    enum {SIZE = 3};
};
template <>
struct JValueTypeTraits<asl::Point4d> {
    typedef asl::Point4d self_type;
    typedef double elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef asl::Vector4d vector_type;
    static std::string Postfix() {
        return "d";
    }
    static std::string Name() {
        return "Point4d";
    }
    enum {SIZE = 4};
};

template <>
struct JValueTypeTraits<asl::Point2f> {
    typedef asl::Point2f self_type;
    typedef float elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef asl::Vector2f vector_type;
    static std::string Postfix() {
        return "f";
    }
    static std::string Name() {
        return "Point2f";
    }
    enum {SIZE = 2};
};
template <>
struct JValueTypeTraits<asl::Point3f> {
    typedef asl::Point3f self_type;
    typedef float elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef asl::Vector3f vector_type;
    static std::string Postfix() {
        return "f";
    }
    static std::string Name() {
        return "Point3f";
    }
    enum {SIZE = 2};
};
template <>
struct JValueTypeTraits<asl::Point4f> {
    typedef asl::Point4f self_type;
    typedef float elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef asl::Vector4f vector_type;
    static std::string Postfix() {
        return "f";
    }
    static std::string Name() {
        return "Point4f";
    }
    enum {SIZE = 4};
};

template <>
struct JValueTypeTraits<asl::Point2i> {
    typedef asl::Point2i self_type;
    typedef int elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef asl::Vector2i vector_type;
    static std::string Postfix() {
        return "i";
    }
    static std::string Name() {
        return "Point2i";
    }
    enum {SIZE = 2};
};
template <>
struct JValueTypeTraits<asl::Point3i> {
    typedef asl::Point3i self_type;
    typedef int elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef asl::Vector3i vector_type;
    static std::string Postfix() {
        return "i";
    }
    static std::string Name() {
        return "Point3i";
    }
    enum {SIZE = 3};
};
template <>
struct JValueTypeTraits<asl::Point4i> {
    typedef asl::Point4i self_type;
    typedef int elem_type;
    typedef dom::VectorValue<self_type> wrapper_type;
    typedef asl::Vector4i vector_type;
    static std::string Postfix() {
        return "i";
    }
    static std::string Name() {
        return "Point4i";
    }
    enum {SIZE = 4};
};

template <class NATIVE_VECTOR>
struct JSVector  {
    enum { SIZE = NATIVE_VECTOR::SIZE };
    typedef typename asl::Ptr<typename dom::ValueWrapper<NATIVE_VECTOR>::Type, dom::ThreadingModel>
        NativeValuePtr;

    typedef typename JValueTypeTraits<NATIVE_VECTOR>::elem_type ELEM;
    typedef typename JValueTypeTraits<NATIVE_VECTOR>::vector_type vector_type;
    typedef JSVector Base;
    typedef JSBool (*NativePropertyGetter)(const NATIVE_VECTOR & theNative, JSContext *cx, jsval *vp);
    typedef JSBool (*NativePropertySetter)(const NATIVE_VECTOR & theNative, JSContext *cx, jsval *vp);

    static
    JSVector * getJSWrapperPtr(JSContext *cx, JSObject *obj);

    static
    JSVector & getJSWrapper(JSContext *cx, JSObject *obj);

    static
    bool matchesClassOf(JSContext *cx, jsval theVal); 

    // This functions must be called only on JSObjects containing the correct
    // native ValueBase pointer in their private field
    static
    const NATIVE_VECTOR & getNativeRef(JSContext *cx, JSObject *obj);
    static
    NATIVE_VECTOR & openNativeRef(JSContext *cx, JSObject *obj);
    static
    void closeNativeRef(JSContext *cx, JSObject *obj);

    virtual NATIVE_VECTOR & openNative();
    virtual void closeNative();
    virtual const NATIVE_VECTOR & getNative() const;
    virtual NativeValuePtr & getOwner();
    virtual const NativeValuePtr & getOwner() const;

    static JSBool
    toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval); 

    static JSBool
    clone(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

    static JSBool
    callMethod(void (NATIVE_VECTOR::*theMethod)(const vector_type &),
         JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
    
    enum PropertyEnum { PROP_LENGTH = -100, PROP_VALUE };

    static JSBool
    add(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
    
    static JSBool
    sub(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval); 
    
    static JSBool
    mult(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval); 
    
    static JSBool
    div(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval); 
    
    static JSBool
    getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);
    
    static JSBool
    setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

    static JSBool
    newResolve(JSContext *cx, JSObject *obj, jsval id, uintN flags, JSObject **objp);

    // --- if char* comparison is used instead of strcmp()
    // the following is needed to make sure the char* are the same
    // in all modules (see Singletonmanager.h)
    // If not, It doesn't hurt either.
    class JSClassNameSingleton :
        public asl::Singleton<JSClassNameSingleton>
    {
        friend class asl::SingletonManager;
    public:
        const std::string & getMyClassName() const {
            return _myClassName;
        }
    protected:
        JSClassNameSingleton() :
            _myClassName(JValueTypeTraits<NATIVE_VECTOR>::Name())
        {}
    private:
        std::string _myClassName;
    };

    static const char * ClassName(); 
    
    static void finalizeImpl(JSContext *cx, JSObject *obj);

    static JSFunctionSpec * Functions(); 

    static JSPropertySpec * Properties(); 

    static JSConstIntPropertySpec * ConstIntProperties(); 

    static JSPropertySpec * StaticProperties(); 

    static JSFunctionSpec * StaticFunctions(); 

    // --
    class JSClassSingleton :
        public asl::Singleton<JSClassSingleton>
    {
    public:
        friend class asl::SingletonManager;
        JSClass * getClass() {
            return &_myJSClass;
        }
    private:
        JSClassSingleton() {
            JSClass myTempClass = {
                ClassName(),                           // const char          *name;
                JSCLASS_HAS_PRIVATE |                  // uint32              flags;
                //JSCLASS_HAS_PRIVATE |               /* objects have private slot */
                //JSCLASS_NEW_ENUMERATE |             /* has JSNewEnumerateOp hook */
                //JSCLASS_NEW_RESOLVE //|                 /* has JSNewResolveOp hook */
                //JSCLASS_PRIVATE_IS_NSISUPPORTS |    /* private is (nsISupports *) */
                //JSCLASS_SHARE_ALL_PROPERTIES |      /* all properties are SHARED */
                //JSCLASS_NEW_RESOLVE_GETS_START      //JSNewResolveOp gets starting
                                                      //object in prototype chain
                                                      //passed in via *objp in/out
                                                      //parameter */
                0,
                /* Mandatory non-null function pointer members. */
                NoisyAddProperty,    // JSPropertyOp        addProperty;
                NoisyDelProperty,    // JSPropertyOp        delProperty;
                JSVector::getProperty,         // JSPropertyOp        getProperty;
                JSVector::setProperty,         // JSPropertyOp        setProperty;
                NoisyEnumerate,      // JSEnumerateOp       enumerate;
                NoisyResolve,        // JSResolveOp         resolve;
                //(JSResolveOp)newResolve,  // JSResolveOp         resolve;
                NoisyConvert,        // JSConvertOp         convert;
                JSVector::finalizeImpl,           // JSFinalizeOp        finalize;

                JSCLASS_NO_OPTIONAL_MEMBERS
                /* Optionally non-null members start here. */
                                    // JSGetObjectOps      getObjectOps;
                                    // JSCheckAccessOp     checkAccess;
                                    // JSNative            call;
                                    // JSNative            construct;
                                    // JSXDRObjectOp       xdrObject;
                                    // JSHasInstanceOp     hasInstance;
                                    // JSMarkOp            mark;
                                    // jsword      ;
            };
            _myJSClass = myTempClass;
        }
    private:
        JSClass _myJSClass;
    };

    static JSClass * Class();

    JSVector();

    JSVector(JSContext * cx, NativeValuePtr theValue);
    
    JSVector(const asl::FixedVector<SIZE,jsdouble> & theArgs);
    
    JSVector(const NATIVE_VECTOR & theVector);
    
    JSVector(const JSVector & theVector);
    
    JSVector & operator=(const JSVector & theVector);

    ~JSVector(); 

    static
    JSObject * Construct(JSContext *cx, jsval theVectorArgument); 
    
    static
    JSObject * Construct(JSContext *cx, uintN argc, jsval *argv);

    static
    JSObject * Construct(JSContext *cx, const NATIVE_VECTOR & theVector);

    static
    JSObject * Construct(JSContext *cx, NativeValuePtr theVector); 
private:
    static JSBool
    Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);
public:
    static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject); 
private:
    NativeValuePtr _myOwner;
};
#define DEFINE_VECTOR_CLASS_TRAITS(TYPE) \
template <> \
struct JSClassTraits<TYPE> : public JSClassTraitsWrapper<TYPE, JSVector<TYPE> > {};

DEFINE_VECTOR_CLASS_TRAITS(asl::Vector2f)
DEFINE_VECTOR_CLASS_TRAITS(asl::Vector3f)
DEFINE_VECTOR_CLASS_TRAITS(asl::Vector4f)

DEFINE_VECTOR_CLASS_TRAITS(asl::Vector2d)
DEFINE_VECTOR_CLASS_TRAITS(asl::Vector3d)
DEFINE_VECTOR_CLASS_TRAITS(asl::Vector4d)

DEFINE_VECTOR_CLASS_TRAITS(asl::Vector2i)
DEFINE_VECTOR_CLASS_TRAITS(asl::Vector3i)
DEFINE_VECTOR_CLASS_TRAITS(asl::Vector4i)

DEFINE_VECTOR_CLASS_TRAITS(asl::Point2f)
DEFINE_VECTOR_CLASS_TRAITS(asl::Point3f)
DEFINE_VECTOR_CLASS_TRAITS(asl::Point4f)

DEFINE_VECTOR_CLASS_TRAITS(asl::Point2d)
DEFINE_VECTOR_CLASS_TRAITS(asl::Point3d)
DEFINE_VECTOR_CLASS_TRAITS(asl::Point4d)

DEFINE_VECTOR_CLASS_TRAITS(asl::Point2i)
DEFINE_VECTOR_CLASS_TRAITS(asl::Point3i)
DEFINE_VECTOR_CLASS_TRAITS(asl::Point4i)

} // namespace

#endif
