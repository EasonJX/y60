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
//   $RCSfile: JSScene.cpp,v $
//   $Author: christian $
//   $Revision: 1.6 $
//   $Date: 2005/04/28 17:12:58 $
//
//
//=============================================================================

#include "JSVector.h"
#include "JSNode.h"
#include "JSNodeList.h"
#include "JSLine.h"
#include "JSSphere.h"
#include "JSMatrix.h"
#include "JSintersection_functions.h"
#include "JSScene.h"
#include "JSApp.h"
#include <iostream>

#include <y60/IProgressNotifier.h>
#include <y60/Body.h>
#include <y60/modelling_functions.h>
#include <asl/Logger.h>

using namespace y60;
using namespace asl;
using namespace std;

namespace jslib {

class ProgressCallback : public y60::IProgressNotifier {
    public:
        ProgressCallback(JSContext * theContext, JSObject * theTarget, const string & theHandler) 
            : _jsContext(theContext), _jsTarget(theTarget), _handlerName(theHandler) 
        {};
        
        virtual void onProgress(float theProgress, const std::string & theMessage="") {
            jsval myVal;
            JSBool bOK = JS_GetProperty(_jsContext, _jsTarget, _handlerName.c_str(), &myVal);
            if (myVal == JSVAL_VOID) {
                AC_WARNING << "JS event handler for event '" << _handlerName << "' missing." << endl;
                return;
            }

            // call the function
            jsval argv[2], rval;
            argv[0] = as_jsval(_jsContext, theProgress);
            argv[1] = as_jsval(_jsContext, theMessage);

            JSBool ok = JSA_CallFunctionName(_jsContext, _jsTarget, _handlerName.c_str(), 2, argv, &rval);
            if (!ok) {
                AC_WARNING << "could not call progress callback " << _handlerName << endl;
            }
            return;
        }
    private:
        JSContext * _jsContext;
        JSObject * _jsTarget;
        string _handlerName;
};

    
typedef y60::Scene NATIVE;

JSBool
toString(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Returns a string representation of the Scene's DOM.");
    DOC_END;
    std::string myStringRep = asl::as_string(JSScene::getJSWrapper(cx,obj).getNative().getSceneDom());
    JSString * myString = JS_NewStringCopyN(cx,myStringRep.c_str(),myStringRep.size());
    *rval = STRING_TO_JSVAL(myString);
    return JS_TRUE;
}

static JSBool
intersectBodies(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Returns the intersections of a body with a given line, line segment or ray.");
    DOC_PARAM("Body", DOC_TYPE_NODE);
    DOC_PARAM("Line", DOC_TYPE_LINE);
    DOC_RESET;
    DOC_PARAM("Body", DOC_TYPE_NODE);
    DOC_PARAM("LineSegment", DOC_TYPE_LINESEGMENT);
    DOC_RESET;
    DOC_PARAM("Body", DOC_TYPE_NODE);
    DOC_PARAM("Ray", DOC_TYPE_RAY);
    DOC_RVAL("IntersectionInfoVector", DOC_TYPE_ARRAY)
    DOC_END;
    try {
        ensureParamCount(argc, 2);
        dom::NodePtr myBodies;
        y60::IntersectionInfoVector myIntersections;
        convertFrom(cx, argv[0], myBodies);
        if (JSLineSegment::matchesClassOf(cx, argv[1])) {
            asl::LineSegment<float> myStick;
            convertFrom(cx, argv[1], myStick);
            y60::Scene::intersectBodies(myBodies, myStick, myIntersections);
        } else if (JSLine::matchesClassOf(cx, argv[1])) {
            asl::Line<float> myLine;
            convertFrom(cx, argv[1], myLine);
            y60::Scene::intersectBodies(myBodies, myLine, myIntersections);
        } else if (JSRay::matchesClassOf(cx, argv[1])) {
            asl::Ray<float> myRay;
            convertFrom(cx, argv[1], myRay);
            y60::Scene::intersectBodies(myBodies, myRay, myIntersections);
        } else {
            JS_ReportError(cx,"JSScene::intersectBodies: bad argument type #1");
            return JS_FALSE;
        }
        *rval = as_jsval(cx, myIntersections);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
collideWithBodies(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Returns the collisions detected by recursively testing a sphere with a given motion vector and a body node.");
    DOC_PARAM("Body", DOC_TYPE_NODE);
    DOC_PARAM("Sphere", DOC_TYPE_SPHERE);
    DOC_PARAM("MotionVector", DOC_TYPE_VECTOR3F);
    DOC_RVAL("CollisionInfoVector", DOC_TYPE_ARRAY)
    DOC_END;
    try {
        ensureParamCount(argc, 3);
        dom::NodePtr myBodies;
        asl::Sphere<float> mySphere;
        asl::Vector3<float> myMotion;
        y60::CollisionInfoVector myCollisions;
        convertFrom(cx, argv[0], myBodies);
        convertFrom(cx, argv[1], mySphere);
        convertFrom(cx, argv[2], myMotion);
        y60::Scene::collideWithBodies(myBodies, mySphere, myMotion, myCollisions);
        *rval = as_jsval(cx, myCollisions);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
collideWithBodiesOnce(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Returns the first collision detected by recursively testing a sphere with a given motion vector and a body node.");
    DOC_PARAM("Body", DOC_TYPE_NODE);
    DOC_PARAM("Sphere", DOC_TYPE_SPHERE);
    DOC_PARAM("MotionVector", DOC_TYPE_VECTOR3F);
    DOC_RVAL("CollisionInfo", DOC_TYPE_OBJECT)
    DOC_END;
    try {
        ensureParamCount(argc, 3);
        dom::NodePtr myBodies;
        asl::Sphere<float> mySphere;
        asl::Vector3<float> myMotion;
        y60::CollisionInfo myCollision;
        convertFrom(cx, argv[0], myBodies);
        convertFrom(cx, argv[1], mySphere);
        convertFrom(cx, argv[2], myMotion);
        y60::Scene::collideWithBodies(myBodies, mySphere, myMotion, myCollision);
        *rval = as_jsval(cx, myCollision);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

static JSBool
update(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Updates the components of the scene that are set in theUpdateFlags.");
    DOC_END;
    JSBool mySuccess = Method<NATIVE>::call(&NATIVE::update,cx,obj,argc,argv,rval);
    *rval = as_jsval(cx, mySuccess);
    return JS_TRUE;
}

static JSBool
bodyVolume(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Calculates the volume of a given body.");
    DOC_PARAM("BodyNode", DOC_TYPE_NODE);
    DOC_RVAL("Volume", DOC_TYPE_FLOAT)
    DOC_END;
    try {
        ensureParamCount(argc, 1);
        JSScene::OWNERPTR myNative;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        dom::NodePtr myBodyNode;
        convertFrom(cx, argv[0], myBodyNode);

        double myVolume = myBodyNode->getFacade<y60::Body>()->calculateVolume(*myNative);
        *rval = as_jsval(cx, myVolume);
        return JS_TRUE;

    } HANDLE_CPP_EXCEPTION;
}

static JSBool
CreateLambertMaterial(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Creates an untextured lambert shaded materail.");
    DOC_PARAM_OPT("theDiffuseColor", DOC_TYPE_VECTOR4F, "[1,1,1,1]");
    DOC_PARAM_OPT("theAmbientColor", DOC_TYPE_VECTOR4F, "[0,0,0,1]");
    DOC_RVAL("theMaterialNode", DOC_TYPE_NODE)
    DOC_END;
    try {
        ensureParamCount(argc, 0, 2);
        JSScene::OWNERPTR myNative;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        dom::NodePtr myResult;
        if (argc == 0) {
            myResult = y60::createLambertMaterial(myNative);
        } else {
            asl::Vector4f myDiffuseColor;
            convertFrom(cx, argv[0], myDiffuseColor);
            if (argc == 1) {
                myResult = y60::createLambertMaterial(myNative, myDiffuseColor);
            } else {
                asl::Vector4f myAmbientColor;
                convertFrom(cx, argv[1], myAmbientColor);
                myResult = y60::createLambertMaterial(myNative, myDiffuseColor, myAmbientColor);
            }
        }

        *rval = as_jsval(cx, myResult);
        return JS_TRUE;

    } HANDLE_CPP_EXCEPTION;
}

static JSBool
CreateColorMaterial(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Creates an untextured and unlit colored materail.");
    DOC_PARAM_OPT("theColor", DOC_TYPE_VECTOR4F, "[1,1,1,1]");
    DOC_RVAL("theMaterialNode", DOC_TYPE_NODE)
    DOC_END;
    try {
        ensureParamCount(argc, 0, 1);
        JSScene::OWNERPTR myNative;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        dom::NodePtr myResult;
        if (argc == 0) {
            myResult = y60::createColorMaterial(myNative);
        } else {
            asl::Vector4f myColor;
            convertFrom(cx, argv[0], myColor);
            myResult = y60::createColorMaterial(myNative, myColor);
        }

        *rval = as_jsval(cx, myResult);
        return JS_TRUE;

    } HANDLE_CPP_EXCEPTION;
}

static JSBool
CreateQuadShape(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Creates a single quad.");
    DOC_PARAM("theMaterial", DOC_TYPE_NODE);
    DOC_PARAM("theTopLeftCorner", DOC_TYPE_VECTOR3F);
    DOC_PARAM("theBottomRightCorner", DOC_TYPE_VECTOR3F);
    DOC_RVAL("theQuadShape", DOC_TYPE_NODE)
    DOC_END;
    try {
        ensureParamCount(argc, 3, 3);
        JSScene::OWNERPTR myNative;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        dom::NodePtr myMaterial;
        convertFrom(cx, argv[0], myMaterial);
        asl::Vector3f myTopLeftCorner;
        convertFrom(cx, argv[1], myTopLeftCorner);
        asl::Vector3f myBottomRightCorner;
        convertFrom(cx, argv[2], myBottomRightCorner);
        dom::NodePtr myResult = y60::createQuad(myNative, myMaterial->getAttributeString("id"), myTopLeftCorner, myBottomRightCorner);
        *rval = as_jsval(cx, myResult);
        return JS_TRUE;

    } HANDLE_CPP_EXCEPTION;
}

static JSBool
CreateBody(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Creates a body");
    DOC_PARAM("theShape", DOC_TYPE_NODE);
    DOC_PARAM_OPT("theParent", DOC_TYPE_NODE, "toplevel node");
    DOC_RVAL("theBody", DOC_TYPE_NODE)
    DOC_END;
    try {
        ensureParamCount(argc, 1, 2);
        JSScene::OWNERPTR myNative;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        dom::NodePtr myShape;
        convertFrom(cx, argv[0], myShape);            
        dom::NodePtr myParent = myNative->getWorldRoot();
        if (argc == 2) {
            convertFrom(cx, argv[1], myParent);            
        } 

        dom::NodePtr myResult = y60::createBody(myParent, myShape->getAttributeString("id"));
        *rval = as_jsval(cx, myResult);
        return JS_TRUE;

    } HANDLE_CPP_EXCEPTION;
}

static JSBool
getWorldSize(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Returns the world's size including camera position.");
    DOC_RVAL("Distance", DOC_TYPE_FLOAT)
    DOC_END;
    return Method<NATIVE>::call(&NATIVE::getWorldSize,cx,obj,argc,argv,rval);
}

static JSBool
clear(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Empties the scene.");
    DOC_END;
    return Method<NATIVE>::call(&NATIVE::clear,cx,obj,argc,argv,rval);
}

static JSBool
createStubs(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Create a minimal empty world.");
    DOC_END;
    try {
        ensureParamCount(argc, 0, 0);
        JSScene::OWNERPTR myNative;
        convertFrom(cx, OBJECT_TO_JSVAL(obj), myNative);
        PackageManagerPtr myPackageManager = JSApp::getPackageManager();
        myNative->createStubs(myPackageManager);
        return JS_TRUE;
    } HANDLE_CPP_EXCEPTION;
}

enum PropertyNumbers {
    PROP_dom = -100,
    PROP_cameras,
    PROP_world,
    PROP_canvases,     
    PROP_canvas,       
    PROP_materials,    
    PROP_lightsources, 
    PROP_animations,   
    PROP_characters,   
    PROP_shapes,       
    PROP_images,       
    PROP_MATERIALS,
    PROP_SHAPES,
    PROP_ANIMATIONS,
    PROP_ANIMATIONS_LOAD,
    PROP_WORLD,
    PROP_IMAGES,
    PROP_ALL,
    PROP_END
};

JSScene::~JSScene() {
    AC_TRACE << "JSScene DTOR " << this << endl; 
}

JSFunctionSpec *
JSScene::StaticFunctions() {
    AC_DEBUG << "Accessing Static Functions to Class '"<<ClassName()<<"'"<<endl;
    static JSFunctionSpec myFunctions[] = {
        // name                    native          nargs
        {"intersectBodies",        intersectBodies,        2},
        {"collideWithBodies",      collideWithBodies,      3},
        {"collideWithBodiesOnce",  collideWithBodiesOnce,  3},
        {0}
    };
    return myFunctions;
}
JSFunctionSpec *
JSScene::Functions() {
    AC_DEBUG << "Accessing functions for class '"<<ClassName()<<"'"<<endl;
    static JSFunctionSpec myFunctions[] = {
        /* name                 native               nargs    */
        {"update",              update,              1},
        {"clear",               clear,               0},
        {"bodyVolume",          bodyVolume,          1},
        {"save",                save,                2},
        {"createStubs",         createStubs,         0},
        {"createLambertMaterial", CreateLambertMaterial, 2},
        {"createColorMaterial",   CreateColorMaterial,   1},
        {"createBody",            CreateBody,            2},
        {"createQuadShape",       CreateQuadShape,       3},
        {0}
    };
    return myFunctions;
}

#define DEFINE_SCENE_FLAG(NAME) { #NAME, PROP_ ## NAME , y60::Scene::NAME }

JSConstIntPropertySpec *
JSScene::ConstIntProperties() {

    static JSConstIntPropertySpec myProperties[] = {
        DEFINE_SCENE_FLAG(MATERIALS),
        DEFINE_SCENE_FLAG(SHAPES),
        DEFINE_SCENE_FLAG(ANIMATIONS),
        DEFINE_SCENE_FLAG(ANIMATIONS_LOAD),
        DEFINE_SCENE_FLAG(WORLD),
        DEFINE_SCENE_FLAG(IMAGES),
        DEFINE_SCENE_FLAG(ALL),
        {0}
    };
    return myProperties;
};

JSPropertySpec *
JSScene::Properties() {
    static JSPropertySpec myProperties[] = {
        {"dom",          PROP_dom,          JSPROP_ENUMERATE | JSPROP_PERMANENT|JSPROP_SHARED | JSPROP_READONLY},
        {"cameras",      PROP_cameras,      JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED | JSPROP_READONLY},
        {"world",        PROP_world,        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED | JSPROP_READONLY},        
        {"canvases",     PROP_canvases,     JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED | JSPROP_READONLY},
        {"canvas",       PROP_canvas,       JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED | JSPROP_READONLY},
        {"materials",    PROP_materials,    JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED | JSPROP_READONLY},
        {"lightsources", PROP_lightsources, JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED | JSPROP_READONLY},
        {"animations",   PROP_animations,   JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED | JSPROP_READONLY},
        {"characters",   PROP_characters,   JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED | JSPROP_READONLY},
        {"shapes",       PROP_shapes,       JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED | JSPROP_READONLY},
        {"images",       PROP_images,       JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_SHARED | JSPROP_READONLY},
        {0}
    };
    return myProperties;
}

JSPropertySpec *
JSScene::StaticProperties() {
    static JSPropertySpec myProperties[] = {{0}};
    return myProperties;
}

// getproperty handling
JSBool
JSScene::getPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
        case PROP_cameras:
            {
                dom::NodePtr myNode = dom::NodePtr(new dom::Node());
                myNode->childNodes().setShell(0); // make sure our list does not clear parent pointers of appended nodes
                getNative().collectCameras(myNode->childNodes());
                *vp = as_jsval(cx, myNode, &myNode->childNodes());
                return JS_TRUE;
            }
        case PROP_dom:
            *vp = as_jsval(cx, getNative().getSceneDom()->childNode(SCENE_ROOT_NAME));
            return JS_TRUE;
        case PROP_world:
            *vp = as_jsval(cx, getNative().getWorldRoot());
            return JS_TRUE;
        case PROP_canvases:
            *vp = as_jsval(cx, getNative().getCanvasRoot());
            return JS_TRUE;
        case PROP_canvas:
            *vp = as_jsval(cx, getNative().getCanvasRoot()->childNode(CANVAS_NODE_NAME));
            return JS_TRUE;
        case PROP_materials:
            *vp = as_jsval(cx, getNative().getSceneDom()->childNode(SCENE_ROOT_NAME)->childNode(MATERIAL_LIST_NAME));
            return JS_TRUE;
        case PROP_lightsources:
            *vp = as_jsval(cx, getNative().getSceneDom()->childNode(SCENE_ROOT_NAME)->childNode(LIGHTSOURCE_LIST_NAME));
            return JS_TRUE;
        case PROP_animations:
            *vp = as_jsval(cx, getNative().getSceneDom()->childNode(SCENE_ROOT_NAME)->childNode(ANIMATION_LIST_NAME));
            return JS_TRUE;
        case PROP_characters:
            *vp = as_jsval(cx, getNative().getSceneDom()->childNode(SCENE_ROOT_NAME)->childNode(CHARACTER_LIST_NAME));
            return JS_TRUE;
        case PROP_shapes:
            *vp = as_jsval(cx, getNative().getShapesRoot());
            return JS_TRUE;
        case PROP_images:
            *vp = as_jsval(cx, getNative().getImagesRoot());
            return JS_TRUE;
        default:
            JS_ReportError(cx,"JSScene::getProperty: index %d out of range", theID);
            return JS_FALSE;
    }
};

// setproperty handling
JSBool
JSScene::setPropertySwitch(unsigned long theID, JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    switch (theID) {
        case 0:
        default:
            JS_ReportError(cx,"JSScene::setPropertySwitch: index %d out of range", theID);
            return JS_FALSE;
    }
};

JSBool
JSScene::Constructor(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Creates a scene from the given file or an empty scene if no file is given.");
    DOC_RESET;
    DOC_PARAM("Filename", DOC_TYPE_STRING);
    DOC_RESET;
    DOC_PARAM("Filename", DOC_TYPE_STRING);
    DOC_PARAM("Target for ProgressCallback", DOC_TYPE_STRING);
    DOC_PARAM("Progress callback function name", DOC_TYPE_STRING);
    DOC_END;
    if (JSA_GetClass(cx,obj) != Class()) {
        JS_ReportError(cx,"Constructor for %s  bad object; did you forget a 'new'?",ClassName());
        return JS_FALSE;
    }
    OWNERPTR myNewPtr = OWNERPTR(new y60::Scene());
    JSScene * myNewObject=new JSScene(myNewPtr, &(*myNewPtr));

    try {
        Ptr<ProgressCallback> myCallback; 
        if (argc >= 3) {
            JSObject * myTarget = JSVAL_TO_OBJECT(argv[1]);
            string myHandler;
            convertFrom(cx, argv[2], myHandler);
            myCallback = Ptr<ProgressCallback>(new ProgressCallback(cx, myTarget, myHandler));
        }

        if (argc == 0 || argv[0] == JSVAL_NULL) {
            AC_INFO << "no filename, creating scene stubs";
            PackageManagerPtr myPackageManager = JSApp::getPackageManager();
            myNewPtr->createStubs(myPackageManager);
        } else {
            std::string myFilename = as_string(cx, argv[0]);
            PackageManagerPtr myPackageManager = JSApp::getPackageManager();
            AC_INFO << "Loading Scene " << myFilename;
            myPackageManager->add(asl::getDirName(myFilename));
            myNewPtr->load(getBaseName(myFilename), myPackageManager, myCallback);
        }

        if (myNewObject) {
            JS_SetPrivate(cx,obj,myNewObject);
            return JS_TRUE;
        }
    } HANDLE_CPP_EXCEPTION;


    JS_ReportError(cx,"JSScene::Constructor: cannot be constructed.");
    return JS_FALSE;
}


JSObject *
JSScene::initClass(JSContext *cx, JSObject *theGlobalObject) {
    JSObject * myClass = Base::initClass(cx, theGlobalObject, ClassName(), Constructor,
        Properties(), Functions(), ConstIntProperties(), 0, StaticFunctions());
    DOC_CREATE(JSScene);
    return myClass;
}

bool convertFrom(JSContext *cx, jsval theValue, asl::Ptr<y60::Scene> & theScene) {
    if (JSVAL_IS_OBJECT(theValue)) {
        JSObject * myArgument;
        if (JS_ValueToObject(cx, theValue, &myArgument)) {
            if (JSA_GetClass(cx,myArgument) == JSClassTraits<y60::Scene>::Class()) {
                theScene = JSClassTraits<y60::Scene>::getNativeOwner(cx,myArgument);
                return true;
            }
        }
    }
    return false;
}

jsval as_jsval(JSContext *cx, asl::Ptr<y60::Scene> theScene) {
    if (theScene) {
        JSObject * myReturnObject = JSScene::Construct(cx, theScene, &(*theScene));
        return OBJECT_TO_JSVAL(myReturnObject);
    }
    return JSVAL_NULL;
}

JSBool
JSScene::save(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("Saves the scene to a file.");
    DOC_PARAM("Filename", DOC_TYPE_STRING);
    DOC_PARAM("BinaryFlag", DOC_TYPE_BOOLEAN);
    DOC_END;
    ensureParamCount(argc, 1, 2);
    return Method<JSScene::NATIVE>::call(&JSScene::NATIVE::save,cx,obj,argc,argv,rval);
}

} // namespace

