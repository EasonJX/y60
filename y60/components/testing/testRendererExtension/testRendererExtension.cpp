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
//
// Description: TODO  
//
// Last Review: NEVER, NOONE
//
//  review status report: (perfect, ok, fair, poor, disaster, notapplicable, unknown)
//    usefullness            : unknown
//    formatting             : unknown
//    documentation          : unknown
//    test coverage          : unknown
//    names                  : unknown
//    style guide conformance: unknown
//    technical soundness    : unknown
//    dead code              : unknown
//    readability            : unknown
//    understandabilty       : unknown
//    interfaces             : unknown
//    confidence             : unknown
//    integration            : unknown
//    dependencies           : unknown
//    cheesyness             : unknown
//
//    overall review status  : unknown
//
//    recommendations: 
//       - unknown
// __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
*/
//
//    $RCSfile: testRendererExtension.cpp,v $
//
//     $Author: martin $
//
//   $Revision: 1.10 $
//
// Description:
//
//=============================================================================

#include <asl/base/PlugInBase.h>
#include <y60/jslib/IRendererExtension.h>
#include <y60/jslib/AbstractRenderWindow.h>
#include <y60/jsbase/JSScriptablePlugin.h>
#include <y60/jsbase/QuitFlagSingleton.h>

#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace asl;
using namespace jslib;
using namespace y60;

typedef vector<string> MyVectorOfString;

#define ASSERT(x) if (!(x)) {cerr << "Assertion fialed: " << #x << " " << __FILE__ << " " <<__LINE__ << endl; exit(1);}

class TestRendererExtension :
    public PlugInBase,
    public IRendererExtension,
    public IScriptablePlugin
{
public:
    TestRendererExtension(asl::DLHandle theDLHandle);

    JSFunctionSpec * Functions();

	void onUpdateSettings(dom::NodePtr theConfiguration) {};
    void onGetProperty(const std::string & thePropertyName,
                     PropertyValue & theReturnValue) const {};

    void onSetProperty(const std::string & thePropertyName,
                     const PropertyValue & thePropertyValue) {};

	const char * ClassName() {
	    static const char * myClassName = "testRendererExtension";
	    return myClassName;
	}

    void onStartup(jslib::AbstractRenderWindow * theWindow);
    bool onSceneLoaded(jslib::AbstractRenderWindow * theWindow);

	void handle(AbstractRenderWindow * theWindow, y60::EventPtr theEvent);
    void onFrame(AbstractRenderWindow * theWindow , double t);
    void onPreRender(AbstractRenderWindow * theRenderer);
    void onPostRender(AbstractRenderWindow * theRenderer);
    void foo();
protected:
    void checkOrderOfCallbacks();
    MyVectorOfString _myStrings;
};

TestRendererExtension :: TestRendererExtension(DLHandle theDLHandle) :
    PlugInBase(theDLHandle),
    IRendererExtension("TestRendererExtension")
{}

void
TestRendererExtension :: foo() {
    cerr << "foo" << endl;
    _myStrings.push_back("foo");
}

void
TestRendererExtension :: onStartup(jslib::AbstractRenderWindow * theWindow) {
    cerr << "onStartup" << endl;
    _myStrings.push_back("onStartup");
}

bool
TestRendererExtension :: onSceneLoaded(jslib::AbstractRenderWindow * theWindow) {
    cerr << "onSceneLoaded" << endl;
    _myStrings.push_back("onSceneLoaded");
    return false;
}

void
TestRendererExtension :: handle(AbstractRenderWindow * theWindow, y60::EventPtr theEvent) {
    cerr << "handle" << endl;
    //_myStrings.push_back("handle");
}

void
TestRendererExtension :: onFrame(AbstractRenderWindow * theWindow , double t) {
    cerr << "onFrame" << endl;
    _myStrings.push_back("onFrame");
}

void
TestRendererExtension :: onPreRender(AbstractRenderWindow * theRenderer) {
    cerr << "onPreRender" << endl;
    _myStrings.push_back("onPreRender");
}

void
TestRendererExtension :: onPostRender(AbstractRenderWindow * theRenderer) {
    cerr << "onPostRender" << endl;
    _myStrings.push_back("onPostRender");

    if (_myStrings.size() >  2 + 10 * 3) {
        checkOrderOfCallbacks();
    }
}

void
TestRendererExtension :: checkOrderOfCallbacks() {
    ASSERT(_myStrings[0] == "onStartup");
    ASSERT(_myStrings[1] == "onSceneLoaded");
    for(int i=0; i<30; i+=3) {
        ASSERT(_myStrings[i+5] == "onFrame");
        ASSERT(_myStrings[i+6] == "onPreRender");
        ASSERT(_myStrings[i+7] == "onPostRender");
    }
    cerr << "testRendererExtension succeeded, quit" << endl;
    jslib::QuitFlagSingleton::get().setQuitFlag(true); //exit(0);
}

static JSBool
Foo(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    DOC_BEGIN("");
    DOC_END;
    asl::Ptr<TestRendererExtension> myNative = getNativeAs<TestRendererExtension>(cx, obj);
    if (myNative) {
        myNative->foo();
    } else {
        ASSERT(myNative);
    }
    return JS_TRUE;
}

JSFunctionSpec *
TestRendererExtension::Functions() {
    AC_DEBUG << "TestRendererExtension::Functions";
    static JSFunctionSpec myFunctions[] = {
        {"foo", Foo, 0},
        {0}
    };
    return myFunctions;
}


extern "C"
EXPORT PlugInBase* testRendererExtension_instantiatePlugIn(DLHandle myDLHandle) {
    return new TestRendererExtension(myDLHandle);
}

