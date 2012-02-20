
#define WINVER 0x0601 // set this so we get the Win7 SDK defines. 

#include "WMTouchPlugin.h"
#include "wmtoucheventxsd.h"

#include <asl/base/Auto.h>
#include <asl/base/ThreadLock.h>
#include <asl/base/error_functions.h>

#include <asl/dom/Nodes.h>

#include <y60/base/DataTypes.h>
#include <y60/base/SettingsParser.h>
#include <y60/input/GenericEvent.h>
#include <y60/input/GenericEventSourceFilter.h>
#include <y60/jsbase/JSWrapper.h>
#include <y60/jsbase/JSNode.h>

#include <SDL/SDL_syswm.h>
#include <SDL/SDL.h>

#define DB(x) //x


// some defines so we can look up Win7 functions at runtime
typedef BOOL (__stdcall *RegisterTouchWindow_M)(HWND, ULONG); 
typedef BOOL (__stdcall *UnregisterTouchWindow_M)(HWND); 
typedef BOOL (__stdcall *GetTouchInputInfo_M)(HTOUCHINPUT, UINT, PTOUCHINPUT, unsigned int); 
typedef BOOL (__stdcall *CloseTouchInputHandle_M)(HTOUCHINPUT); 

namespace y60 {

RegisterTouchWindow_M RegisterTouchWindow; 
UnregisterTouchWindow_M UnregisterTouchWindow; 
GetTouchInputInfo_M GetTouchInputInfo; 
CloseTouchInputHandle_M CloseTouchInputHandle; 

using namespace asl;
using namespace dom;

static WMTouchPlugin* ourWMTouchPluginInstance = 0;
HHOOK WMTouchPlugin::_msgHook = 0;

WMTouchPlugin::WMTouchPlugin(DLHandle myDLHandle)
        : PlugInBase(myDLHandle),
          GenericEventSourceFilter(),
          _myEventSchemaDocument(new Document(y60::ourwmtoucheventxsd)),
          _myEventValueFactory(new ValueFactory()),
          _isRegistered(false),
          IRendererExtension("WMTouch")
{
    HMODULE	hMod = LoadLibrary(_T("User32.dll"));
    RegisterTouchWindow = (RegisterTouchWindow_M) GetProcAddress(hMod, "RegisterTouchWindow");
    if (RegisterTouchWindow == 0) {
        AC_ERROR << "can't find Win7 RegisterTouchWindow - Windows 7 Touch Events not supported!";
        return;
    }
    UnregisterTouchWindow = (UnregisterTouchWindow_M) GetProcAddress(hMod, "UnregisterTouchWindow");
    GetTouchInputInfo = (GetTouchInputInfo_M) GetProcAddress(hMod, "GetTouchInputInfo");
    CloseTouchInputHandle = (CloseTouchInputHandle_M) GetProcAddress(hMod, "CloseTouchInputHandle");
    registerStandardTypes(*_myEventValueFactory);
    registerSomTypes(*_myEventValueFactory);

}

WMTouchPlugin::~WMTouchPlugin() { 
    if (_isRegistered && UnregisterTouchWindow) {
        UnregisterTouchWindow(findSDLWindow());
    }
}

HWND
WMTouchPlugin::findSDLWindow() {
    SDL_SysWMinfo myInfo;
    SDL_VERSION(&myInfo.version);
    int rc = SDL_GetWMInfo(&myInfo);
    if (rc != 1) {
        AC_WARNING << "Failed to locate main application window: " << SDL_GetError() << ". Disabling WMTouch.";
    }
    return myInfo.window;
}

void 
WMTouchPlugin::setup(HWND theWindow) {
    if (theWindow && RegisterTouchWindow) {
        BOOL bRet = RegisterTouchWindow(theWindow, 0L);
        if (bRet) {
            AC_DEBUG << "RegisterTouchWindow succeeded: " << bRet;
            _isRegistered = true;
        } else {
            AC_WARNING << errorDescription(lastError()); 
        }
        // register message hook
        SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc,(HINSTANCE) NULL, GetCurrentThreadId());    
    }
};

LRESULT WINAPI 
WMTouchPlugin::GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode < 0) {
		return CallNextHookEx(0, nCode, wParam, lParam);
	}
    AC_TRACE << "GetMsgProc called nCode=" << nCode << ", wParam:" << wParam << ", lParam:" << lParam;
	switch (nCode) { 
	    case HC_ACTION:
            if (wParam == PM_REMOVE) {
                LPMSG msg = (LPMSG)lParam;
                switch (msg->message) {
                    case WM_TOUCH:
                        AC_TRACE << "WM_TOUCH recv. hwnd:" << msg->hwnd << ", wParam:" << msg->wParam << ", lParam" << msg->lParam;
                        ourWMTouchPluginInstance->onTouch(msg->hwnd, msg->wParam, msg->lParam);
                        break;

                }
            }
            break;
    };
    return CallNextHookEx(0, nCode, wParam, lParam);
};

void
WMTouchPlugin::onTouch(HWND hWnd, WPARAM wParam, LPARAM lParam) {
    BOOL bHandled = FALSE;
    UINT cInputs = LOWORD(wParam);
    PTOUCHINPUT pInputs = new TOUCHINPUT[cInputs];
    if (GetTouchInputInfo((HTOUCHINPUT)lParam, cInputs, pInputs, sizeof(TOUCHINPUT))){
        AC_DEBUG << "found " << cInputs << " Cursors!";
        for (UINT i=0; i < cInputs; i++){
            const TOUCHINPUT & ti = pInputs[i];
            // create a y60 event for each InputInfo
            GenericEventPtr myEvent(new GenericEvent("onWMTouch", _myEventSchemaDocument, _myEventValueFactory));
            NodePtr myNode = myEvent->getNode();

            myNode->appendAttribute<int>("id", ti.dwID);
            myNode->appendAttribute<Vector2f>("position", product(Vector2f(static_cast<float>(ti.x), static_cast<float>(ti.y)), 0.01f));
            if (ti.dwMask & TOUCHINPUTMASKF_CONTACTAREA) {
                myNode->appendAttribute<Vector2f>("contactarea", product(Vector2f(static_cast<float>(ti.cxContact), static_cast<float>(ti.cyContact)), 0.01f));
            }
            myNode->appendAttribute<bool>("inrange", (ti.dwFlags & TOUCHEVENTF_INRANGE) != 0);
            myNode->appendAttribute<bool>("primary", (ti.dwFlags & TOUCHEVENTF_PRIMARY) != 0);
            myNode->appendAttribute<bool>("nocoalesce", (ti.dwFlags & TOUCHEVENTF_NOCOALESCE) != 0);
            myNode->appendAttribute<bool>("palm", (ti.dwFlags & TOUCHEVENTF_PALM) != 0);
            // Note: we seem to be getting duplicate events, probably due to the way we are hooking the msg queue.
            // So we filter the events here. Only queue a DOWN event if the cursor wasn't down yet,
            // and only queue an UP or MOVE event if the cursor was down.
            if ((ti.dwFlags & TOUCHEVENTF_DOWN) != 0 && _downCursors.find(ti.dwID) == _downCursors.end()) {
                myNode->appendAttribute<DOMString>("type", "down");
                _queuedEvents.push_back(myEvent);
                _downCursors.insert(ti.dwID);
            } else if ((ti.dwFlags & TOUCHEVENTF_UP) != 0 && _downCursors.find(ti.dwID) != _downCursors.end()) {
                myNode->appendAttribute<DOMString>("type", "up");
                _queuedEvents.push_back(myEvent);
                _downCursors.erase(ti.dwID);
            } else if ((ti.dwFlags & TOUCHEVENTF_MOVE) != 0 && _downCursors.find(ti.dwID) != _downCursors.end()) {
                _queuedEvents.push_back(myEvent);
                myNode->appendAttribute<DOMString>("type", "move");
            }
        }            
        bHandled = TRUE;
    }else{
        /* handle the error here */
        AC_ERROR << errorDescription(lastError());
    }
    // clean up
    delete [] pInputs;

    if (bHandled){
        // if you handled the message, close the touch input handle and return
        CloseTouchInputHandle((HTOUCHINPUT)lParam);
        return;
    }
};

NodePtr 
WMTouchPlugin::getEventSchema() {
    return _myEventSchemaDocument;
};
    
EventPtrList 
WMTouchPlugin::poll() {
    EventPtrList deliveredEvents;
    for (EventPtrList::iterator it = _queuedEvents.begin(); it != _queuedEvents.end(); ++it) {
        deliveredEvents.push_back(*it);
    }
    _queuedEvents.clear();
    AC_DEBUG << "delivering " << deliveredEvents.size() << " events";
    return deliveredEvents;
};

// implementation of js interface


static WMTouchPlugin*
GetInstance() {
    if(ourWMTouchPluginInstance) {
        return ourWMTouchPluginInstance;
    } else {
        throw Exception("Calling WMTouch static function before plugin initialization.");
    }
}

enum WMTouchPropertyNumbers {
    PROP_eventSchema = -100
};

JSFunctionSpec *
WMTouchPlugin::StaticFunctions() {
    static JSFunctionSpec myFunctions[] = {
        // {"listenToUDP", ListenToUDP, 0},
        {0}
    };
    return myFunctions;
}

static JSBool
GetStaticProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    WMTouchPlugin* myPlugin = GetInstance();

    int myID = JSVAL_TO_INT(id);
    switch(myID) {
    case PROP_eventSchema:
        *vp = jslib::as_jsval(cx, 0);
        break;
    default:
        JS_ReportError(cx, "WMTouch::getStaticProperty: index %d out of range", myID);
        return JS_FALSE;
    }
    return JS_TRUE;
}

JSPropertySpec *
WMTouchPlugin::StaticProperties() {
    static JSPropertySpec myProperties[] = {
        {"eventSchema", PROP_eventSchema, JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, GetStaticProperty, 0},
        {0}
    };
    return myProperties;
}

}

extern "C"
EXPORT asl::PlugInBase* WMTouch_instantiatePlugIn(asl::DLHandle myDLHandle) {
    if(y60::ourWMTouchPluginInstance) {
        AC_FATAL << "Plugging WMTouch more than once";
        return 0;
    } else {
        AC_TRACE << "Instantiating WMTouch plugin";
        y60::ourWMTouchPluginInstance = new y60::WMTouchPlugin(myDLHandle);
        return y60::ourWMTouchPluginInstance;
    }
}
