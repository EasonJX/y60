
#include <TuioClient.h>
#include <TuioListener.h>

#include <asl/base/Auto.h>
#include <asl/base/ThreadLock.h>

#include <asl/dom/Nodes.h>

#include <y60/base/DataTypes.h>
#include <y60/input/IEventSource.h>
#include <y60/input/GenericEvent.h>
#include <y60/jsbase/IScriptablePlugin.h>
#include <y60/jsbase/JSWrapper.h>
#include <y60/jsbase/JSNode.h>

#include "tuioeventxsd.h"

namespace y60 {

using namespace asl;
using namespace dom;
using namespace TUIO;

class TUIOPlugin : public PlugInBase,
                   public y60::IEventSource,
                   public jslib::IScriptablePlugin,
                   public TuioListener
{

private:
    typedef std::pair<const char*, TuioCursor*> CursorEvent;
    typedef std::vector<CursorEvent> CursorEventList;

public:

    TUIOPlugin(DLHandle myDLHandle)
        : PlugInBase(myDLHandle),
          _myEventSchemaDocument(new Document(y60::ourtuioeventxsd)),
          _myEventValueFactory(new ValueFactory())
    {
        registerStandardTypes(*_myEventValueFactory);
        registerSomTypes(*_myEventValueFactory);
    }

    ~TUIOPlugin()
    { }

// IScriptablePlugin

    const char * ClassName() {
        return "TUIOClient";
    }

    JSFunctionSpec *StaticFunctions();
    JSPropertySpec *StaticProperties();



// IEventSource

    void init() {
    }

    EventPtrList poll() {
        AutoLocker<ThreadLock> l(_myDeliveryMutex);
        if(_myUndeliveredCursors.size() > 0) {
            EventPtrList myEvents;

            CursorEventList::iterator it;
            for(it = _myUndeliveredCursors.begin(); it != _myUndeliveredCursors.end(); ++it) {
                CursorEvent & myEvent = *it;
                myEvents.push_back(convertCursorEvent(myEvent.first, myEvent.second));
                delete myEvent.second;
                myEvent.second = 0;
            }

            _myUndeliveredCursors.clear();

            return myEvents;
        } else {
            return EventPtrList();
        }
    }

// TuioListener

    virtual void addTuioObject(TuioObject *tobj) {
    }

    virtual void updateTuioObject(TuioObject *tobj) {
    }

    virtual void removeTuioObject(TuioObject *tobj) {
    }

    virtual void addTuioCursor(TuioCursor *myCursor) {
        handleCursor(myCursor, "add");
    }

    virtual void updateTuioCursor(TuioCursor *myCursor) {
        handleCursor(myCursor, "update");
    }

    virtual void removeTuioCursor(TuioCursor *myCursor) {
        handleCursor(myCursor, "remove");
    }

    virtual void refresh(TuioTime ftime) {
    }

// implementation of js interface

    void listenToUDP(Unsigned16 thePort) {
        AC_INFO << "Listening for TUIO messages on UDP port " << thePort;
        TuioClient *myClient = new TuioClient(thePort);
        myClient->connect();
        myClient->addTuioListener(this);
        _myClients.push_back(myClient);
    }

    NodePtr getEventSchema() {
        return _myEventSchemaDocument;
    }

// reused stuff

protected:

    void handleCursor(TuioCursor *myCursor, const char *myEventType) {
        AC_TRACE << "Handling cursor " << myEventType << " for "
                 << myCursor->getSessionID() << ":" << myCursor->getCursorID();

        AutoLocker<ThreadLock> l(_myDeliveryMutex);

        TuioCursor *myCursorCopy = new TuioCursor(myCursor);

        _myUndeliveredCursors.push_back(CursorEvent(myEventType, myCursorCopy));
    }

    GenericEventPtr convertCursorEvent(const char *myEventType, TuioCursor *myCursor) {
        GenericEventPtr myEvent(new GenericEvent("onTuioEvent", _myEventSchemaDocument, _myEventValueFactory));
        NodePtr myNode = myEvent->getNode();

        myNode->appendAttribute<DOMString>("type", myEventType);

        myNode->appendAttribute<int>("id", myCursor->getSessionID());

        TuioTime myStartTime = myCursor->getStartTime();
        myNode->appendAttribute<double>("start_time", myStartTime.getSeconds() + (myStartTime.getMicroseconds() / 1000000.0));

        TuioTime myValueTime = myCursor->getTuioTime();
        myNode->appendAttribute<double>("value_time", myValueTime.getSeconds() + (myValueTime.getMicroseconds() / 1000000.0));

        myNode->appendAttribute<Vector2f>("position", Vector2f(myCursor->getX(), myCursor->getY()));
        myNode->appendAttribute<Vector2f>("velocity", Vector2f(myCursor->getXSpeed(), myCursor->getYSpeed()));

        myNode->appendAttribute<double>("speed", myCursor->getMotionSpeed());
        myNode->appendAttribute<double>("acceleration", myCursor->getMotionAccel());

        return myEvent;
    }

private:

    DocumentPtr                 _myEventSchemaDocument;
    asl::Ptr<dom::ValueFactory> _myEventValueFactory;

    ThreadLock      _myDeliveryMutex;
    CursorEventList _myUndeliveredCursors;

    std::vector<TuioClient*> _myClients;

};



static TUIOPlugin* ourTuioPluginInstance = 0;

static TUIOPlugin*
GetInstance() {
    if(ourTuioPluginInstance) {
        return ourTuioPluginInstance;
    } else {
        throw Exception("Calling TUIO static function before plugin initialization.");
    }
}

static JSBool
ListenToUDP(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {
    TUIOPlugin* myPlugin = GetInstance();

    Unsigned16 myPort = 3333;

    if(argc > 0) {
        if(!jslib::convertFrom(cx, argv[0], myPort)) {
            JS_ReportError(cx, "TUIO::listenToUDP - expected port as first argument");
            return JS_FALSE;
        }
    }

    myPlugin->listenToUDP(myPort);

    return JS_TRUE;
}

enum TUIOPropertyNumbers {
    PROP_eventSchema = -100
};

JSFunctionSpec *
TUIOPlugin::StaticFunctions() {
    static JSFunctionSpec myFunctions[] = {
        {"listenToUDP", ListenToUDP, 0},
        {0}
    };
    return myFunctions;
}

static JSBool
GetStaticProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp) {
    TUIOPlugin* myPlugin = GetInstance();

    int myID = JSVAL_TO_INT(id);
    switch(myID) {
    case PROP_eventSchema:
        *vp = jslib::as_jsval(cx, myPlugin->getEventSchema());
        break;
    default:
        JS_ReportError(cx, "TUIO::getStaticProperty: index %d out of range", myID);
        return JS_FALSE;
    }
    return JS_TRUE;
}

JSPropertySpec *
TUIOPlugin::StaticProperties() {
    static JSPropertySpec myProperties[] = {
        {"eventSchema", PROP_eventSchema, JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, GetStaticProperty, 0},
        {0}
    };
    return myProperties;
}

}

extern "C"
EXPORT asl::PlugInBase* TUIOClient_instantiatePlugIn(asl::DLHandle myDLHandle) {
    if(y60::ourTuioPluginInstance) {
        AC_FATAL << "Plugging TUIO more than once";
        return 0;
    } else {
        AC_TRACE << "Instantiating TUIO plugin";
        y60::ourTuioPluginInstance = new y60::TUIOPlugin(myDLHandle);
        return y60::ourTuioPluginInstance;
    }
}
