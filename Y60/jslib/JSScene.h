//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _Y60_JSLIB_JSSCENE_INCLUDED_
#define _Y60_JSLIB_JSSCENE_INCLUDED_

#include <Y60/JSWrapper.h>

#include <y60/Scene.h>


namespace jslib {

    class JSScene : public JSWrapper<y60::Scene, asl::Ptr<y60::Scene, dom::ThreadingModel>, StaticAccessProtocol> {
    JSScene() {}
public:
    virtual ~JSScene();
    typedef y60::Scene NATIVE;
    typedef asl::Ptr<y60::Scene, dom::ThreadingModel> OWNERPTR;
    typedef JSWrapper<NATIVE,OWNERPTR,StaticAccessProtocol> Base;
    static const char * ClassName() {
        return "Scene";
    }
    static JSFunctionSpec * Functions();
    static JSFunctionSpec * StaticFunctions();

    static JSConstIntPropertySpec * ConstIntProperties();
    static JSPropertySpec * Properties();
    static JSPropertySpec * StaticProperties();

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
        JSObject * Construct(JSContext *cx, asl::Ptr<y60::Scene,dom::ThreadingModel> theScene, y60::Scene * theNative) {
        return Base::Construct(cx, theScene, theNative);
    }
    JSScene(asl::Ptr<y60::Scene, dom::ThreadingModel> theScene, y60::Scene * theNative)
        : Base(theScene, theNative)
    {}

    static JSObject * initClass(JSContext *cx, JSObject *theGlobalObject);

    static JSBool
    save(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval);

    /*
    static JSObject *
    ConstructWithArgs(JSContext * cx, OWNERPTR theOwner, NATIVE * theNative, 
            uintN argc, jsval * argv);
    */

    
private:
    JSBool JSScene::getStatistics(JSContext *cx, jsval *vp);

};

template <>
struct JSClassTraits<y60::Scene>
    : public JSClassTraitsWrapper<y60::Scene, JSScene> {};

bool convertFrom(JSContext *cx, jsval theValue, asl::Ptr<y60::Scene, dom::ThreadingModel> & theScene);

jsval as_jsval(JSContext *cx, JSScene::OWNERPTR theScene);

}

#endif
