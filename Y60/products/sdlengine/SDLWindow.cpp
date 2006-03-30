//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "SDLWindow.h"
#include "SDLTextRenderer.h"
#include <y60/JScppUtils.h>

#include <SDL/SDL.h>

#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#include <SDL/SDL_syswm.h>

#include <iostream>
#include <asl/numeric_functions.h>
#include <asl/GeometryUtils.h>
#include <asl/Box.h>

#include <y60/JSBox.h>
#include <y60/JSLine.h>
#include <y60/JSTriangle.h>
#include <y60/JSSphere.h>
#include <y60/JSVector.h>
#include <y60/JSMatrix.h>
#include <y60/JSNode.h>
#include <y60/JSNodeList.h>
#include <y60/JSScriptablePlugin.h>
#include <y60/JSApp.h>
#include <y60/JScppUtils.h>
#include <y60/GLAlloc.h>
#include <y60/ShaderLibrary.h>

using namespace std;
using namespace y60;

#define DB(x) //x

DEFINE_EXCEPTION(SDLWindowException, asl::Exception);

asl::Ptr<SDLWindow>
SDLWindow::create() {
    asl::Ptr<SDLWindow> newWindow = asl::Ptr<SDLWindow>(new SDLWindow());
    newWindow->setSelf(newWindow);
    return newWindow;
}

SDLWindow::SDLWindow() :
    AbstractRenderWindow(SDLApp::ShellErrorReporter),
    _myWindowTitle("Y60 Renderer"),
    _myWindowPosX(100), _myWindowPosY(100),
    _myWindowInitFlag(false),
    _myFullscreenFlag(false),
    _myWinDecoFlag(true),
    _myShowMouseCursorFlag(true),
    _myCaptureMouseCursorFlag(false),
    _myScreen(0),
    _myInitialWidth(800),
    _myInitialHeight(600),
    _myUserDefinedCursor(0),
    _myStandardCursor(0),
    _myAutoPauseFlag(false),
    _mySwapInterval(0),
    _myLastSwapCounter(0)
{
    setGLContext(GLContextPtr(new GLContext()));
}

SDLWindow::~SDLWindow() {
    AC_DEBUG << "SDLWindow::~SDLWindow";
    SDL_Quit();
}

TTFTextRendererPtr
SDLWindow::createTTFRenderer() {
    return TTFTextRendererPtr(new SDLTextRenderer());
}

void
SDLWindow::activateGLContext() {
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        ensureSDLSubsystem();
#ifdef WIN32
        unsigned int myFlags = SDL_OPENGL | SDL_NOFRAME;
#else
        unsigned int myFlags = SDL_OPENGL;
#endif
        SDL_SetVideoMode(1, 1, 32, myFlags);
    }
}

void
SDLWindow::deactivateGLContext() {
}

int
SDLWindow::getWidth() const {
    return _myScreen ? _myScreen->w : 0;
}

int
SDLWindow::getHeight() const {
    return _myScreen ? _myScreen->h : 0;
}

void
SDLWindow::postRender() {
    AbstractRenderWindow::postRender();
    MAKE_SCOPE_TIMER(SDL_GL_SwapBuffers);
#ifdef LINUX
    if (_mySwapInterval) {
        unsigned counter;
        glXWaitVideoSyncSGI(_mySwapInterval, 0, &counter);
        if (_myLastSwapCounter == 0) {
            _myLastSwapCounter = counter;
        }
        if ((counter - _myLastSwapCounter) != _mySwapInterval) {
            AC_DEBUG << "Missed frame diff=" << (counter - _myLastSwapCounter);
        }
        _myLastSwapCounter = counter;
    }
#endif
    SDL_GL_SwapBuffers();
}

void
SDLWindow::onResize(Event & theEvent) {
    WindowEvent & myWindowEvent = dynamic_cast<WindowEvent&>(theEvent);
    AC_DEBUG << "Window Resize Event: " << myWindowEvent.width << "x" << myWindowEvent.height;
#ifdef WIN32
    if (!_myWinDecoFlag) {
        // Sorry, in case of a non decorated window SDL seems to use getClientRect for getting the window size.
        // This is wrong, because decorations are excluded, and so height and width are not right
        // and the window is too small, use GetWindowRect instead. (VS)
        SDL_SysWMinfo wminfo;
        SDL_VERSION(&wminfo.version);
        LPCTSTR myRenderGirlWindowName = _myWindowTitle.c_str();
        HWND myRenderGirlWindow = FindWindow(0, myRenderGirlWindowName);
        if (myRenderGirlWindow) {
            RECT myRect;
            GetWindowRect(myRenderGirlWindow, &myRect);
            setVideoMode(myRect.right - myRect.left, myRect.bottom - myRect.top);
        } else {
            setVideoMode(myWindowEvent.width, myWindowEvent.height, false);
        }
    } else {
        setVideoMode(myWindowEvent.width, myWindowEvent.height, false);
    }
#else
    setVideoMode(myWindowEvent.width, myWindowEvent.height, false);
#endif
    AbstractRenderWindow::onResize(theEvent);
}

void
SDLWindow::setVideoMode(unsigned theTargetWidth, unsigned theTargetHeight,
                        bool theFullscreenFlag)
{
    DB(AC_TRACE << "setVideoMode(" << theTargetWidth << ", " << theTargetHeight <<
        ", Fullscreen: " << theFullscreenFlag << ")");
    DB(AC_TRACE << "_myWindowInitFlag: " << _myWindowInitFlag);
    if (!_myWindowInitFlag) {
        // window not yet set up, defer to later
        _myInitialWidth = theTargetWidth;
        _myInitialHeight = theTargetHeight;
    } else {
        unsigned int myFlags = SDL_OPENGL;

        if (!_myWinDecoFlag) {
            myFlags |= SDL_NOFRAME;
        }
        if (theFullscreenFlag) {
            myFlags |= SDL_FULLSCREEN;
        } else {
            myFlags |= SDL_RESIZABLE;
        }

        if (theTargetWidth == 0) {
            theTargetWidth = getWidth();
            AC_DEBUG << "keeping width=" << theTargetWidth;
        }
        if (theTargetHeight == 0) {
            theTargetHeight = getHeight();
            AC_DEBUG << "keeping height=" << theTargetHeight;
        }

#ifdef LINUX
        SDL_SysWMinfo wminfo;
        SDL_VERSION(&wminfo.version);
        if (SDL_GetWMInfo(&wminfo) >= 0) {
            wminfo.info.x11.lock_func();
            XSync(wminfo.info.x11.display, true);
            wminfo.info.x11.unlock_func();
        }
#endif

        DB(AC_TRACE << "SDL_SetVideoMode(" << theTargetWidth << ", " << theTargetHeight <<
                ", FS=" << theFullscreenFlag << ", Deco=" << _myWinDecoFlag << ")" << endl);
        if ((_myScreen = SDL_SetVideoMode(theTargetWidth, theTargetHeight, 32, myFlags)) ==
                NULL)
        {
            throw SDLWindowException(string("Couldn't set SDL-GL mode: ") + SDL_GetError(), PLUS_FILE_LINE);
        }

        // if we are resetting the video mode (e.g. Fullscreen toggle), then the GL context
        // will be lost. Reinit GL and setup the textures again
        if (_myRenderer) {
            _myRenderer->initGL();
            ShaderLibraryPtr myShaderLibrary = dynamic_cast_Ptr<ShaderLibrary> (_myRenderer->getShaderLibrary());
			if (myShaderLibrary) {
				myShaderLibrary->reload();
			}
        }
        if (_myScene) {
            _myScene->clearShapes();
            _myScene->updateAllModified();
            _myScene->update(Scene::SHAPES);
            _myScene->getTextureManager()->setupTextures();
        }
    }
    _myFullscreenFlag = theFullscreenFlag;
}

void
SDLWindow::initDisplay() {
    ensureSDLSubsystem();
    _myWindowInitFlag = true;

    setVideoMode(_myInitialWidth, _myInitialHeight, _myFullscreenFlag);
    setWindowTitle("Y60 Renderer");

    if (!_myFullscreenFlag) {
        setPosition(asl::Vector2i(_myWindowPosX, _myWindowPosY));
    }
    //dumpSDLGLParams();

    EventDispatcher::get().addSource(&_mySDLEventSource);
    EventDispatcher::get().addSource(&_myEventRecorder);
    EventDispatcher::get().addSink(&_myEventRecorder);

    // Uncomment this to get output on the events that pass through the event queue.
    // EventDispatcher::get().addSink(&_myEventDumper);
    EventDispatcher::get().addSink(this);

#ifdef LINUX
    if (getenv("__GL_SYNC_TO_VBLANK") == 0) {
        AC_WARNING << "__GL_SYNC_TO_VBLANK not set.";
    }
#endif

    AC_INFO  << "GL Version  : " << glGetString(GL_VERSION);
    AC_INFO  << "   Vendor   : " << glGetString(GL_VENDOR);
    AC_INFO  << "   Renderer : " << glGetString(GL_RENDERER);
    AC_DEBUG << "   Extensions : " << glGetString(GL_EXTENSIONS);

    // retrieve standard cursor
    _myStandardCursor =  SDL_GetCursor();
}

void
SDLWindow::dumpSDLGLParams() {
    int value;
    AC_PRINT << "SDL display parameters used: "<< endl;
    SDL_GL_GetAttribute(SDL_GL_RED_SIZE, &value);
    AC_PRINT << "  SDL_GL_RED_SIZE = " << value << endl;
    SDL_GL_GetAttribute(SDL_GL_GREEN_SIZE,  &value);
    AC_PRINT << "  SDL_GL_GREEN_SIZE = " << value << endl;
    SDL_GL_GetAttribute(SDL_GL_BLUE_SIZE , &value);
    AC_PRINT << "  SDL_GL_BLUE_SIZE = " << value << endl;
    SDL_GL_GetAttribute(SDL_GL_ALPHA_SIZE , &value);
    AC_PRINT << "  SDL_GL_ALPHA_SIZE = " << value << endl;
    SDL_GL_GetAttribute(SDL_GL_DEPTH_SIZE , &value);
    AC_PRINT << "  SDL_GL_DEPTH_SIZE = " << value << endl;
    SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER , &value);
    AC_PRINT << "  SDL_GL_DOUBLEBUFFER = " << value << endl;
    SDL_GL_GetAttribute(SDL_GL_BUFFER_SIZE , &value);
    AC_PRINT << "  SDL_GL_BUFFER_SIZE = " << value << endl;
    SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE , &value);
    AC_PRINT << "  SDL_GL_STENCIL_SIZE = " << value << endl;
    SDL_GL_GetAttribute(SDL_GL_ACCUM_RED_SIZE , &value);
    AC_PRINT << "  SDL_GL_ACCUM_RED_SIZE = " << value << endl;
    SDL_GL_GetAttribute(SDL_GL_ACCUM_GREEN_SIZE , &value);
    AC_PRINT << "  SDL_GL_ACCUM_GREEN_SIZE = " << value << endl;
    SDL_GL_GetAttribute(SDL_GL_ACCUM_BLUE_SIZE , &value);
    AC_PRINT << "  SDL_GL_ACCUM_BLUE_SIZE = " << value << endl;
}

void
SDLWindow::resetCursor() {
    if (_myStandardCursor) {
        SDL_SetCursor(_myStandardCursor);
    }
}

// <CursorDesc data="[]" size="[]", hotSize="[]" />
void
SDLWindow::createCursor(dom::NodePtr & theCursorInfo) {
    if (_myUserDefinedCursor) {
        SDL_FreeCursor(_myUserDefinedCursor);
    }

    //      Data / Mask Resulting pixel on screen
    //      0 / 1 White
    //      1 / 1 Black
    //      0 / 0 Transparent
    //      1 / 0 Inverted color if possible, black if not.
    //
    //      'X' = black
    //      ' ' = transparent
    //      '.' = white
    std::string image = theCursorInfo->getAttributeString("data");

    asl::Vector2i mySize = theCursorInfo->getAttributeValue<asl::Vector2i>("size");
    asl::Vector2i myHotSize = theCursorInfo->getAttributeValue<asl::Vector2i>("hotSize");

    if ( (mySize[0] % 8 !=0) || (mySize[1] % 8 !=0) ||(myHotSize[0] % 8 !=0) ||(myHotSize[0] % 8 !=0)) {
        throw SDLWindowException(string("Sorry, cursor dimensions must be dividable by 8: ") + as_string(*theCursorInfo), PLUS_FILE_LINE);
    }

    if (image.size() != mySize[0] * mySize[1]) {
        throw SDLWindowException(string("Sorry, cursor image data is not correct: ") +  as_string(*theCursorInfo), PLUS_FILE_LINE);
    }
    vector<char> myData(4*mySize[0]);
    vector<char> myMask(4*mySize[0]);

    int myIndex = -1;
    for ( int myRow=0; myRow<mySize[0]; ++myRow ) {
        for ( int myCol=0; myCol<mySize[1]; ++myCol ) {
            if ( myCol % 8 ) {
                myData[myIndex] <<= 1;
                myMask[myIndex] <<= 1;
            } else {
                ++myIndex;
                myData[myIndex] = myMask[myIndex] = 0;
            }
            switch (image[((myRow)*mySize[1])+myCol]) {
                case 'X':
                      myData[myIndex] |= 0x01;
                      myMask[myIndex] |= 0x01;
                      break;
                case '.':
                      myMask[myIndex] |= 0x01;
                      break;
                case ' ':
                      break;
            }
        }
    }
    _myUserDefinedCursor= SDL_CreateCursor((Uint8 *)&(myData[0]), (Uint8 *)&(myMask[0]), mySize[0], mySize[1], myHotSize[0], myHotSize[1]);
    SDL_SetCursor(_myUserDefinedCursor);
}

asl::Vector2i
SDLWindow::getScreenSize(unsigned theScreen) const {
    int myWidth  = -1;
    int myHeight = -1;

#ifdef WIN32
    HWND myDesktopWindow = GetDesktopWindow();
    if (myDesktopWindow) {
        RECT myWindowRect;
        GetWindowRect(myDesktopWindow, &myWindowRect);
        myWidth  = myWindowRect.right - myWindowRect.left;
        myHeight = myWindowRect.bottom - myWindowRect.top;
    }
#else
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (SDL_GetWMInfo(&wminfo) < 0) {
        AC_ERROR << "SDL_GetWMInfo: " << SDL_GetError() << endl;
    } else {
        Display * myDisplay = wminfo.info.x11.display;
        myWidth  = DisplayWidth(myDisplay, theScreen);
        myHeight = DisplayHeight(myDisplay, theScreen);
    }
#endif
    if (myWidth >= 0 && myHeight >= 0) {
        AC_DEBUG << "screen size=" << myWidth << "x" << myHeight;
    }
    return asl::Vector2i(myWidth, myHeight);
}

void SDLWindow::setPosition(asl::Vector2i thePos) {
    _myWindowPosX = thePos[0];
    _myWindowPosY = thePos[1];

    if (_myWindowInitFlag == false) {
        // _myWindowPosX/_myWindowPosY will be used, when window is opened
        return;
    }

    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (SDL_GetWMInfo(&wminfo) < 0) {
        AC_ERROR << "SDL_GetWMInfo: " << SDL_GetError() << endl;
        return;
    }
#ifdef WIN32
    LPCTSTR myRenderGirlWindowName = _myWindowTitle.c_str();
    HWND myRenderGirlWindow = FindWindow(0, myRenderGirlWindowName);
    if (myRenderGirlWindow != wminfo.window) {
        AC_ERROR << "Windows don't match!\n";
        myRenderGirlWindow = wminfo.window;
    }
    if (myRenderGirlWindow) {
        RECT myRect;
        GetWindowRect(myRenderGirlWindow, &myRect);
        MoveWindow(myRenderGirlWindow, thePos[0], thePos[1],
                   myRect.right-myRect.left,
                   myRect.bottom-myRect.top, true);
    }
#else
    wminfo.info.x11.lock_func();
    XMoveWindow(wminfo.info.x11.display, wminfo.info.x11.wmwindow,
                thePos[0], thePos[1]);
    XSync(wminfo.info.x11.display, false);
    wminfo.info.x11.unlock_func();
#endif
}

void SDLWindow::setWindowTitle(const std::string & theTitle) {
    _myWindowTitle = theTitle;
    SDL_WM_SetCaption(theTitle.c_str(), 0);
}

const std::string &
SDLWindow::getWindowTitle() const {
    return _myWindowTitle;
}

void SDLWindow::setWinDeco(bool theWinDecoFlag) {
    if (_myWindowInitFlag && (theWinDecoFlag != _myWinDecoFlag)) {
        _myWinDecoFlag = theWinDecoFlag;
        setVideoMode();
    } else {
        _myWinDecoFlag = theWinDecoFlag;
    }
}

bool SDLWindow::getWinDeco() {
    return _myWinDecoFlag;
}

bool SDLWindow::getFullScreen() {
    return _myFullscreenFlag;
}

void SDLWindow::ensureSDLSubsystem() {
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (SDL_InitSubSystem(SDL_INIT_VIDEO) == -1) {
            throw SDLWindowException(string("Could not init SDL video subsystem: ") + SDL_GetError(), PLUS_FILE_LINE);
        }
        SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
        SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
        /*SDL_GL_SetAttribute( SDL_GL_ACCUM_RED_SIZE, 16 );
        SDL_GL_SetAttribute( SDL_GL_ACCUM_GREEN_SIZE, 16 );
        SDL_GL_SetAttribute( SDL_GL_ACCUM_BLUE_SIZE, 16 );*/
        SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
    }
}

void SDLWindow::setShowMouseCursor(bool theShowMouseCursor) {
    SDL_ShowCursor(theShowMouseCursor);
    _myShowMouseCursorFlag = theShowMouseCursor;
}

void
SDLWindow::setShowTaskbar(bool theFlag) {
#ifdef WIN32
    HWND myWindowHandle = FindWindow("Shell_TrayWnd", 0);
    ShowWindow(myWindowHandle, (theFlag ? SW_SHOW : SW_HIDE));
#endif
}

bool
SDLWindow::getShowTaskbar() const {
#ifdef WIN32
    HWND myWindowHandle = FindWindow("Shell_TrayWnd", 0);
    return (GetWindowLong(myWindowHandle, GWL_STYLE) & WS_VISIBLE);
#else
    return true;
#endif
}

void SDLWindow::setMousePosition(int theX, int theY) {
    SDL_WarpMouse(theX, theY);
}

void
SDLWindow::setCaptureMouseCursor(bool theCaptureFlag) {
    if (theCaptureFlag == _myCaptureMouseCursorFlag) {
        return;
    }

#ifdef WIN32
    if (theCaptureFlag) {
        LPCTSTR myWindowName = _myWindowTitle.c_str();
        HWND myWindow = FindWindow(0, myWindowName);
        if (myWindow == NULL) {
            AC_ERROR << "Unable to find window '" << _myWindowTitle << "'";
            return;
        }
        RECT myRect;
        GetWindowRect(myWindow, &myRect);
        ClipCursor(&myRect);
    } else {
        ClipCursor(NULL); // free to move
    }
#else
    SDL_SysWMinfo wminfo;
    SDL_VERSION(&wminfo.version);
    if (SDL_GetWMInfo(&wminfo) < 0) {
        AC_ERROR << "SDL_GetWMInfo: " << SDL_GetError() << endl;
        return;
    }
    wminfo.info.x11.lock_func();
    if (theCaptureFlag) {
        XGrabPointer(wminfo.info.x11.display, wminfo.info.x11.wmwindow,
                     True, 0, GrabModeAsync, GrabModeAsync,
                     wminfo.info.x11.wmwindow, None, CurrentTime);
    } else {
        XUngrabPointer(wminfo.info.x11.display, CurrentTime);
    }
    XSync(wminfo.info.x11.display, true);
    wminfo.info.x11.unlock_func();
#endif
    _myCaptureMouseCursorFlag = theCaptureFlag;
}

void
SDLWindow::setAutoPause(bool theAutoPauseFlag) {
    _myAutoPauseFlag = theAutoPauseFlag;
}

void
SDLWindow::handle(EventPtr theEvent) {
    MAKE_SCOPE_TIMER(handleEvents);
    switch (theEvent->type) {
        case Event::QUIT:
            _myAppQuitFlag = true;
            break;
        default:
            AbstractRenderWindow::handle(theEvent);
    }
}

void
SDLWindow::onKey(Event & theEvent) {
    KeyEvent & myKeyEvent = dynamic_cast<KeyEvent&>(theEvent);

    // Hardcoded key event hanlder
    if ((myKeyEvent.keyString == "q" && myKeyEvent.modifiers & KEYMOD_CTRL) ||
        (myKeyEvent.keyString == "f4" && myKeyEvent.modifiers & KEYMOD_ALT)) {
        _myAppQuitFlag = true;
    }

    AbstractRenderWindow::onKey(theEvent);
}

namespace jslib {
    //in JSRenderWindow
    jsval as_jsval(JSContext *cx, asl::Ptr<SDLWindow> theOwner);

    jsval as_jsval(JSContext *cx, asl::Ptr<AbstractRenderWindow> theOwner) {
        return as_jsval(cx, dynamic_cast_Ptr<SDLWindow>( theOwner));
    }

}

void
SDLWindow::mainLoop() {
    _myAppQuitFlag = false;
    // do one update before entering the mainloop. This ensures everything
    // is up to date during first onFrame
    if (_myRenderer) {
        _myRenderer->getCurrentScene()->updateAllModified();
    }

    if (_myEventListener && jslib::JSA_hasFunction(_myJSContext, _myEventListener, "onStartMainLoop")) {
        jsval argv[1], rval;
        jslib::JSA_CallFunctionName(_myJSContext, _myEventListener, "onStartMainLoop", 0, argv, &rval);
    }

    while ( ! _myAppQuitFlag ) {
#ifdef WIN32
        bool isOnTop = false;
        if (_myAutoPauseFlag) {
            LPCTSTR myRenderGirlWindowName = _myWindowTitle.c_str();
            HWND myTopWindow        = GetForegroundWindow();
            HWND myRenderGirlWindow = FindWindow(0, myRenderGirlWindowName);
            isOnTop = (myTopWindow == myRenderGirlWindow);
        }
#endif
        onFrame();

        // Call onProtoFrame (a second onframe that can be used to automatically run tutorials)
        if (_myEventListener && jslib::JSA_hasFunction(_myJSContext, _myEventListener, "onProtoFrame")) {
            jsval argv[1], rval;
            argv[0] = jslib::as_jsval(_myJSContext, _myElapsedTime);
            jslib::JSA_CallFunctionName(_myJSContext, _myEventListener, "onProtoFrame", 1, argv, &rval);
        }

        START_TIMER(dispatchEvents);
        EventDispatcher::get().dispatch();
        STOP_TIMER(dispatchEvents);

        START_TIMER(handleRequests);
        _myRequestManager.handleRequests();
        STOP_TIMER(handleRequests);

        if (_myRenderer && _myRenderer->getCurrentScene()) {
            _myRenderer->getCurrentScene()->updateAllModified();

            preRender();
            render();
            postRender();
        }

        asl::AGPMemoryFlushSingleton::get().resetGLAGPMemoryFlush();
#ifdef WIN32
        if (_myAutoPauseFlag && isOnTop == false) {
            unsigned long mySleepDuration = 40; // in millisec
            asl::msleep(mySleepDuration); 
        }
#endif
        if (jslib::JSApp::getQuitFlag() == JS_TRUE) {
            _myAppQuitFlag = true;
        }

        STOP_TIMER(frames);
        asl::getDashboard().cycle();
        START_TIMER(frames);
    }

    jsval argv[1], rval;
    if (jslib::JSA_hasFunction(_myJSContext, _myEventListener, "onExit")) {
        jslib::JSA_CallFunctionName(_myJSContext, _myEventListener, "onExit", 0, argv, &rval);
    }
}

void
SDLWindow::stop() {
    AC_DEBUG << "SDLWindow::stop" << endl;
    _myAppQuitFlag = true;
}

bool
SDLWindow::go() {
    bool myResult = false;
    AC_DEBUG << "SDLWindow::go";
    if (!_myJSContext) {
        AC_WARNING << "SDLWindow::go() - No js context found, please assign an eventHandler to window before calling go()";
        return myResult;
    }
    try {
        AbstractRenderWindow::go();
        mainLoop();
        myResult = true;
    } catch (const asl::Exception & ex) {
        AC_ERROR << "Exception caught in SDLWindow::go(): " << ex;
    } catch (const exception & ex) {
        AC_ERROR << "Exception caught in SDLWindow::go(): " << ex.what();
    } catch (...) {
        AC_ERROR << "Unknown exception in SDLWindow::go()";
    }
    // release mouse capture
    setCaptureMouseCursor(false);
    return myResult;
}

void SDLWindow::setEventRecorderMode(EventRecorder::Mode theMode, bool theDiscardFlag) {
    _myEventRecorder.setMode(theMode, theDiscardFlag);
}

EventRecorder::Mode SDLWindow::getEventRecorderMode() const {
    return _myEventRecorder.getMode();
}

void SDLWindow::loadEvents(const std::string & theFilename) {
    _myEventRecorder.load(theFilename);
}

void SDLWindow::saveEvents(const std::string & theFilename) {
    _myEventRecorder.save(theFilename);
}


void 
SDLWindow::setSwapInterval(unsigned theInterval)
{
    if (!_myRenderer) {
        AC_WARNING << "Sorry, setting the swap interval will take no effect before the renderer is created!";
        return;
    }

#ifdef WIN32
    if (wglSwapIntervalEXT) {
        wglSwapIntervalEXT(theInterval);
        _mySwapInterval = wglGetSwapIntervalEXT();
        if (theInterval != _mySwapInterval) {
            AC_ERROR << "setSwapInterval(): wglSwapInterval is buggy, using " << _mySwapInterval;
        }
    } else {
        AC_WARNING << "setSwapInterval(): wglSwapInterval not supported.";
    }    
#else
    if (glXGetVideoSyncSGI && glXWaitVideoSyncSGI) {
        if (_mySwapInterval == 0 && theInterval != 0) {
            // check if it's working
            unsigned counter0, counter1;
            glXGetVideoSyncSGI(&counter0);
            asl::msleep(20);
            glXGetVideoSyncSGI(&counter1);
            if (counter1 <= counter0) {
                AC_WARNING << "setSwapInterval(): glXGetVideoSyncSGI not working properly (counter1=" << counter1 << ", counter0=" << counter0 << "), disabling";
                theInterval = 0;
            } else {
                AC_INFO << "setSwapInterval(): glXGetVideoSyncSGI working properly";
            }
        }
        _mySwapInterval = theInterval;
    } else {
        AC_WARNING << "setSwapInterval(): glXGetVideoSyncSGI not supported";
        _mySwapInterval = 0;
    }
#endif
}

int
SDLWindow::getSwapInterval() {
    if (!_myRenderer) {
        AC_WARNING << "Sorry, getting the swap interval will no work before the renderer is created!" << endl;
        return 0;
    }

#ifdef WIN32
    if (wglGetSwapIntervalEXT) {
        return wglGetSwapIntervalEXT();
    } else {
        AC_WARNING << "getSwapInterval(): wglSwapInterval Extension not supported.";
    }    
#else
    if (glXGetVideoSyncSGI) {
        return _mySwapInterval;
    } else {
        AC_WARNING << "getSwapInterval(): glXGetVideoSyncSGI not supported";
    }
#endif
    return 0;
}
