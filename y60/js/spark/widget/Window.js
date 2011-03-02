/*jslint nomen: false, bitwise:false*/
/*global spark, use, SceneViewer, window, RenderWindow,
         Logger, Vector2i, Vector2f, Vector3f, Point2f, clone,
         ASS_BASE_EVENT, LEFT_BUTTON, magnitude, distance*/

/**
 * Simple wrapper for the Y60 scene viewer.
 */

use("SceneViewer.js"); // XXX: bad place for this

spark.Window = spark.ComponentClass("Window");

spark.Window.Constructor = function (Protected, theArguments) {
    var Base = {};
    var Public = this;
    Public.Inherit(spark.Stage);
    // Also inherit from SceneViewer
    SceneViewer.prototype.Constructor(this, theArguments);
    
    /////////////////////
    // Private Members //
    /////////////////////

    var MOVE_DISTANCE_THRESHOLD = 0;//0.1;
    var PICK_RADIUS = 1;
    
    var _myCamera            = null;
    var _myWorld             = null;
    var _myPickRadius        = PICK_RADIUS;
    var _myPickList          = {};
    var _myMultitouchCursors = {};
    
    // XXX: a somewhat hackish callback to get a hand on the scene
    //      after the model has been loaded but before any spark components
    //      are added.
    var _mySceneLoadedCallback;
    
    var _myMousePosition     = new Vector2f();
    var _myMouseButtonStates = {};
    var _myMouseFocused      = null;
    var _myMouseCursor       = null;
    var _myKeyboardFocused   = null;
    
    /////////////////////
    // Private Methods //
    /////////////////////
    
    function getWidgetForMultitouchCursor(theCursor, thePosition) {
        var myWidget;
        if (theCursor.grabbed) {
            myWidget = theCursor.grabHolder;
        } else {
            if (theCursor.id in _myPickList) {
                myWidget = _myPickList[theCursor.id];
            } else {
                myWidget = Public.pickWidget(thePosition.x, thePosition.y);
                _myPickList[theCursor.id] = myWidget;
            }
        }
        if (!myWidget) {
            myWidget = Public;
        }
        return myWidget;
    }
    
    function getMultitouchCursor(theId) {
        var myCursor;
        if (theId in _myMultitouchCursors) {
            myCursor = _myMultitouchCursors[theId];
        } else {
            Logger.debug("Cursor " + theId + " added");
            myCursor = new spark.Cursor(theId);
            _myMultitouchCursors[theId] = myCursor;
        }
        return myCursor;
    }
    
    function getSparkConformedCursorId(theEvent, theId) {
        switch (theEvent.callback) {
        case "onASSEvent":
            return "pmtx" + theId;
        case "onTuioEvent":
            return "tuio" + theId;
        case "onGesture":
            if (theEvent.baseeventtype == ASS_BASE_EVENT) {
                return "pmtx" + theId;
            } else {
                return "tuio" + theId;
            }
        default:
            Logger.fatal("Unknown multitouch event type");
            return null;
        }
    }
    
    function getMultitouchCursorPosition(theEvent) {
        var myPosition;
        switch (theEvent.callback) {
        case "onASSEvent":
            myPosition = new Point2f(theEvent.position3D.x, theEvent.position3D.y);
            break;
        case "onTuioEvent":
            myPosition = new Point2f(theEvent.position.x * ((theEvent.toucharea.x > 0) ? theEvent.toucharea.x : Public.width),
                                     theEvent.position.y * ((theEvent.toucharea.y > 0) ? theEvent.toucharea.y : Public.height));
            break;
        case "onGesture":
            if (theEvent.baseeventtype == ASS_BASE_EVENT) {
                myPosition = new Point2f(theEvent.position3D.x, theEvent.position3D.y);
            } else {
                myPosition = new Point2f(theEvent.position3D.x * ((theEvent.toucharea.x > 0) ? theEvent.toucharea.x : Public.width),
                                         theEvent.position3D.y * ((theEvent.toucharea.y > 0) ? theEvent.toucharea.y : Public.height));
            }
            break;
        default:
            Logger.fatal("Unknown multitouch event type");
            return null;
        }
        return myPosition;
    }

    function handleMultitouchEvent(theEvent) {
        if (theEvent.callback == "onASSEvent") {
            spark.proximatrix.onASSEvent(theEvent);
        }

        var myId = getSparkConformedCursorId(theEvent, theEvent.id);
        switch (theEvent.type) {
        case "configure":
            Logger.info("proximatrix got configured");
            break;
        case "add":
        case "move": // proximatrix
        case "update": // tuio        
            var myCursor = getMultitouchCursor(myId);
            var myFocused;
            if (theEvent.type == "add") {
                myCursor.activate();
            } else {
                myFocused = myCursor.focused;
            }
            
            var myPosition = getMultitouchCursorPosition(theEvent);
            
            var myPick = getWidgetForMultitouchCursor(myCursor, myPosition);
            myCursor.update(myPick, myPosition);

            if (theEvent.type == "add") {
                Logger.debug("Cursor " + myId + " appears in " + myPick);
                var myAppear = null;
                if (theEvent.tuiotype == TUIO_OBJECT_CLASS) {
                    myAppear = new spark.ObjectEvent(spark.ObjectEvent.APPEAR, myCursor, theEvent);
                } else {
                    myAppear = new spark.CursorEvent(spark.CursorEvent.APPEAR, myCursor);
                }
                myPick.dispatchEvent(myAppear);
            }
            if (myCursor.active && (myPick != myFocused)) {
                Logger.debug("Cursor " + myId + " focuses " + myPick
                             + (myFocused ? ", leaving " + myFocused : ""));

                if (myFocused) {
                    var myLeave = null;
                    if (theEvent.tuiotype == TUIO_OBJECT_CLASS) {
                        myLeave = new spark.ObjectEvent(spark.ObjectEvent.LEAVE, myCursor, theEvent);
                    } else {
                        myLeave = new spark.CursorEvent(spark.CursorEvent.LEAVE, myCursor);
                    }
                    myFocused.dispatchEvent(myLeave);
                }
                var myEnter = null;
                if (theEvent.tuiotype == TUIO_OBJECT_CLASS) {
                    myEnter = new spark.ObjectEvent(spark.ObjectEvent.ENTER, myCursor, theEvent);
                } else {
                    myEnter = new spark.CursorEvent(spark.CursorEvent.ENTER, myCursor);
                }
                myPick.dispatchEvent(myEnter);
            }

            if (myCursor.active && (theEvent.type == "move" || theEvent.type == "update")) {
                
                var myMoveDistance = distance(myPosition, myCursor.lastStagePosition);
                if (myMoveDistance >= MOVE_DISTANCE_THRESHOLD) {
                    Logger.debug("Cursor " + myId + " moves to " + myPosition + " over " + myPick);
                    var myMove = null;
                    if (theEvent.tuiotype == TUIO_OBJECT_CLASS) {
                        myMove = new spark.ObjectEvent(spark.ObjectEvent.MOVE, myCursor, theEvent);
                    } else {
                        myMove = new spark.CursorEvent(spark.CursorEvent.MOVE, myCursor);
                    }
                    
                    myPick.dispatchEvent(myMove);
                }
            }
            break;
        case "remove":
            if (myId in _myMultitouchCursors) {
                Logger.debug("Cursor " + myId + " removed");
                var myCursor   = _myMultitouchCursors[myId];
                var myPosition = getMultitouchCursorPosition(theEvent);
                var myFocused  = myCursor.focused;

                myCursor.update(myFocused, myPosition);

                if (myFocused) {
                    var myLeave = null;
                    if (theEvent.tuiotype == TUIO_OBJECT_CLASS) {
                        Logger.debug("Object " + myId + " leaves " + myFocused);                    
                        myLeave = new spark.ObjectEvent(spark.ObjectEvent.LEAVE, myCursor, theEvent);
                    } else {
                        Logger.debug("Cursor " + myId + " leaves " + myFocused);                    
                        myLeave = new spark.CursorEvent(spark.CursorEvent.LEAVE, myCursor);
                    }
                    myFocused.dispatchEvent(myLeave);
                    
                    var myVanish = null;
                    if (theEvent.tuiotype == TUIO_OBJECT_CLASS) {
                        Logger.debug("Object " + myId + " vanishes in " + myFocused);
                        myVanish = new spark.ObjectEvent(spark.ObjectEvent.VANISH, myCursor, theEvent);
                    } else {
                        Logger.debug("Cursor " + myId + " vanishes in " + myFocused);
                        myVanish = new spark.CursorEvent(spark.CursorEvent.VANISH, myCursor);
                    }
                    myFocused.dispatchEvent(myVanish);
                }
                myCursor.deactivate();
                delete _myMultitouchCursors[myId];
            }
            break;
        }
    }
    
    ////////////////////
    // Public Methods //
    ////////////////////

    Public.__defineGetter__("title", function ()  {
        return window.title;
    });
    
    Public.__defineSetter__("title", function (theTitle) {
        window.title = theTitle;
    });

    Public.__defineGetter__("multitouchCursors", function ()  {
        return _myMultitouchCursors;
    });

    Base.realize = Public.realize;
    Public.realize = function () {
        window = new RenderWindow();

        window.position = [
            Protected.getNumber("positionX", 0),
            Protected.getNumber("positionY", 0)
        ];

        window.decorations = Protected.getBoolean("decorations", true);
        window.multisamples = Protected.getNumber("multisamples", 0);

        var mySceneFile = Protected.getString("sceneFile", "");
        if (mySceneFile.length > 0) {
            Public.setModelName(mySceneFile);
        }
        Public.setup(Protected.getNumber("width", 640),
                     Protected.getNumber("height", 480),
                     Protected.getBoolean("fullscreen", false),
                     Protected.getString("title", "SPARK Application"));
        
        if (mySceneFile.length > 0 && _mySceneLoadedCallback) {
            _mySceneLoadedCallback(window.scene.dom);
        }

        Public.setMover(null);

        window.showMouseCursor = Protected.getBoolean("mouseCursor", true);
        window.swapInterval = Protected.getNumber("swapInterval", 1);
        var myFixedDeltaT = Number(eval(Protected.getString("fixedDeltaT", "0"))); // so we can say something like "1/30"
        if (myFixedDeltaT) {
            window.fixedFrameTime = myFixedDeltaT;
        }

        // mixed 2D/3D applications might want to keep the original frustum and manage
        // the screenspace transformation themself.
        var mySetupFrustumFlag = Protected.getBoolean("setupFrustum", true);
        if (mySetupFrustumFlag) {
            spark.setupCameraOrtho(window.scene.dom.find(".//camera"), window.width, window.height);
        }

        Protected.updateMouseButtonState(spark.Mouse.PRIMARY,   false);
        Protected.updateMouseButtonState(spark.Mouse.SECONDARY, false);
        Protected.updateMouseButtonState(spark.Mouse.TERTIARY,  false);

        Base.realize(window.scene.world);
    };

    Base.postRealize = Public.postRealize;
    Public.postRealize = function () {
        Base.postRealize();
        if ("layouting" in Public.arguments) {
            print("initializing Layouter");
            use("spark/add-ons/Layouter.js");
            var myLayouter = new Node("<Layouter name='Layouter_" + Public.name + "'/>");
            spark.loadDocument(myLayouter);
        }
    };

    // XXX: Override size, width and height properties inherited via Stage->Widget
    // They do return a boundingbox size which, from class Windows standpoint, is just
    // crap.
    Public.__defineGetter__("size", function () {
        return new Vector2i(window.width, window.height);
    });
    Public.__defineGetter__("width", function () {
        return window.width;
    });
    Public.__defineGetter__("height", function () {
        return window.height;
    });

    Public.pickWidget = function (theX, theY) {
        var myBody     = null;
        if (_myPickRadius === 0) {
            myBody = window.scene.pickBody(theX, theY, window.scene.canvas);
        } else {
            myBody = window.scene.pickBodyBySweepingSphereFromBodies(theX, theY, _myPickRadius, window.scene.canvas);
        }
        if (myBody) {
            var myBodyId = myBody.id;
            if (myBodyId in spark.sceneNodeMap) {
                var myWidget = spark.sceneNodeMap[myBodyId];
                if ("pickWidget" in myWidget) {
                    var myCanvasWidget = myWidget.pickWidget(theX, theY);
                    if (myCanvasWidget) {
                        myWidget = myCanvasWidget;
                    }
                }
                return myWidget;
            }
        }
        return null;
    };
    
    Public.__defineGetter__("mousePosition", function () {
        return _myMousePosition;
    });

    Protected.updateMousePosition = function (theX, theY) {
        _myMousePosition.x = theX;
        _myMousePosition.y = theY;
    };

    Public.__defineGetter__("mouseButtonStates", function () {
        return _myMouseButtonStates;
    });

    Protected.updateMouseButtonState = function (theButton, theState) {
        _myMouseButtonStates[theButton] = theState;
    };

    Public.__defineGetter__("mouseFocused", function () {
        return _myMouseFocused;
    });

    Public.__defineGetter__("keyboardFocused", function () {
        return _myKeyboardFocused;
    });

    Public.focusKeyboard = function (theWidget) {
        _myKeyboardFocused = theWidget;
    };

    Public.__defineGetter__("onSceneLoaded", function () {
        return _mySceneLoadedCallback;
    });

    Public.__defineSetter__("onSceneLoaded", function (f) {
        _mySceneLoadedCallback = f;
    });
    
    Public.__defineGetter__("pickRadius", function () {
        return _myPickRadius;
    });

    Public.__defineSetter__("pickRadius", function (theRadius) {
        _myPickRadius = theRadius;
    });

    //////////////////////////////////////////////////////////////////////
    // Callbacks
    //////////////////////////////////////////////////////////////////////

    //  Will be called before onFrame, has the time since application start
    Public.onProtoFrame = function (theTime) {
        var myEvent = new spark.StageEvent(spark.StageEvent.PROTO_FRAME, Public, theTime);
        Public.dispatchEvent(myEvent);
    };

    //  Will be called first in renderloop, has the time since application start
    Base.onFrame = Public.onFrame;
    Public.onFrame = function (theTime, theDeltaT) {
        Base.onFrame(theTime, theDeltaT);
        if (Public.hasEventListener(spark.StageEvent.FRAME)) {
            var myEvent = new spark.StageEvent(spark.StageEvent.FRAME, Public, theTime, theDeltaT);
            Public.dispatchEvent(myEvent);
        }
        _myPickList = {};
    };

    // Will be called before rendering the frame
    Base.onPreRender = Public.onPreRender;
    Public.onPreRender = function () {
        Base.onPreRender();
        var myEvent = new spark.StageEvent(spark.StageEvent.PRE_RENDER, Public);
        Public.dispatchEvent(myEvent);
    };

    // Will be called after rendering the frame, but before swap buffer
    Base.onPostRender = Public.onPostRender;
    Public.onPostRender = function () {
        Base.onPostRender();
        var myEvent = new spark.StageEvent(spark.StageEvent.POST_RENDER, Public);
        Public.dispatchEvent(myEvent);
    };

    // Will be called on a mouse move
    Base.onMouseMotion = Public.onMouseMotion;
    Public.onMouseMotion = function (theX, theY) {
        Base.onMouseMotion(theX, theY);

        var myButtonStates = clone(_myMouseButtonStates);
        Protected.updateMousePosition(theX, theY);

        var myWidget = Public.pickWidget(theX, theY);

        if (!myWidget) {
            myWidget = Public;
        }

        if (_myMouseCursor) {
            _myMouseCursor.update(myWidget, new Point2f(theX, theY));
            var myMouseCursorMoveEvent = new spark.CursorEvent(spark.CursorEvent.MOVE, _myMouseCursor);
            if (_myMouseCursor.grabbed) {
                _myMouseCursor.grabHolder.dispatchEvent(myMouseCursorMoveEvent);
            } else {
                myWidget.dispatchEvent(myMouseCursorMoveEvent);
            }
        }
        
        if (myWidget != _myMouseFocused) {
            Logger.debug("Mouse focuses " + myWidget +
                         (_myMouseFocused ? ", leaving " + _myMouseFocused : ""));

            if (_myMouseFocused) {
                var myLeaveEvent = new spark.MouseEvent(spark.MouseEvent.LEAVE, theX, theY, 0, 0, null, myButtonStates);
                _myMouseFocused.dispatchEvent(myLeaveEvent);
            }

            if (_myMouseCursor && !_myMouseCursor.grabbed) {
                if (_myMouseFocused) {
                    var myCursorLeave = new spark.CursorEvent(spark.CursorEvent.LEAVE, _myMouseCursor);
                    _myMouseFocused.dispatchEvent(myCursorLeave);
                }

                var myCursorEnter = new spark.CursorEvent(spark.CursorEvent.ENTER, _myMouseCursor);
                myWidget.dispatchEvent(myCursorEnter);
            }
            _myMouseFocused = myWidget;

            var myEnterEvent = new spark.MouseEvent(spark.MouseEvent.ENTER, theX, theY);
            myWidget.dispatchEvent(myEnterEvent);
        }

        Logger.debug("Mouse moves to [" + theX + "," + theY + "] over " + myWidget);
        var myMoveEvent = new spark.MouseEvent(spark.MouseEvent.MOVE, theX, theY);
        myWidget.dispatchEvent(myMoveEvent);
        
        
        _myMouseFocused = myWidget;
    };

    // Will be called on a mouse button
    Base.onMouseButton = Public.onMouseButton;
    Public.onMouseButton = function (theButton, theState, theX, theY) {
        Base.onMouseButton(theButton, theState, theX, theY);

        var myButton = spark.Mouse.buttonFromId(theButton);
        Protected.updateMouseButtonState(myButton, theState);

        var myButtonStates = clone(_myMouseButtonStates);

        Protected.updateMousePosition(theX, theY);

        var myWidget = Public.pickWidget(theX, theY);
        if (!myWidget) {
            myWidget = Public;
        }
        
        var myMouseCursorEvent;
        if (theButton === LEFT_BUTTON) {
            if (theState) {
                _myMouseCursor = new spark.Cursor("mouse-cursor");
                _myMouseCursor.update(myWidget, new Point2f(theX, theY));
                myMouseCursorEvent = new spark.CursorEvent(spark.CursorEvent.APPEAR, _myMouseCursor);
                myWidget.dispatchEvent(myMouseCursorEvent);
            } else {
                _myMouseCursor.update(myWidget, new Point2f(theX, theY));
                myMouseCursorEvent = new spark.CursorEvent(spark.CursorEvent.VANISH, _myMouseCursor);
                if (_myMouseCursor.grabbed) {
                    _myMouseCursor.grabHolder.dispatchEvent(myMouseCursorEvent);
                } else {
                    myWidget.dispatchEvent(myMouseCursorEvent);
                }
                _myMouseCursor = null;
            }
        }

        if (theState) {
            // XXX: click should be more well-defined and button-up-based.
            Logger.debug("Mouse clicks " + myWidget + " with button " + myButton);
            var myClickEvent = new spark.MouseEvent(spark.MouseEvent.CLICK, theX, theY, 0, 0, myButton, myButtonStates);
            myWidget.dispatchEvent(myClickEvent);

            Logger.debug("Mouse " + myButton + " button down on " + myWidget);
            var myDownEvent = new spark.MouseEvent(spark.MouseEvent.BUTTON_DOWN, theX, theY, 0, 0, myButton, myButtonStates);
            myWidget.dispatchEvent(myDownEvent);
        } else {
            Logger.debug("Mouse " + myButton + " button up on " + myWidget);
            var myUpEvent = new spark.MouseEvent(spark.MouseEvent.BUTTON_UP, theX, theY, 0, 0, myButton, myButtonStates);
            myWidget.dispatchEvent(myUpEvent);
        }
    };

    // Will be called on a keyboard event
    Base.onKey = Public.onKey;
    Public.onKey = function (theKey, theKeyState, theX, theY,
                             theShiftFlag, theControlFlag, theAltFlag) {
        Base.onKey(theKey, theKeyState, theX, theY, theShiftFlag, theControlFlag, theAltFlag);

        if (_myKeyboardFocused) {
            var myModifiers =
                (theShiftFlag ? spark.Keyboard.SHIFT : 0) | (theControlFlag ? spark.Keyboard.CTRL : 0) | (theAltFlag ? spark.Keyboard.ALT : 0);
            var myType = theKeyState ? spark.KeyboardEvent.KEY_DOWN : spark.KeyboardEvent.KEY_UP;
            var myEvent = new spark.KeyboardEvent(myType, theKey, myModifiers);
            Logger.debug("Key " + myEvent.keyString + " " + (theKeyState ? "down" : "up") + " on " + _myKeyboardFocused);
            _myKeyboardFocused.dispatchEvent(myEvent);
        }
    };

    // Will be called on a mouse wheel event
    Base.onMouseWheel = Public.onMouseWheel;
    Public.onMouseWheel = function (theDeltaX, theDeltaY) {
        Base.onMouseWheel(theDeltaX, theDeltaY);

        var myButtonStates = clone(_myMouseButtonStates);
        if (_myMouseFocused) {
            Logger.debug("Mouse scrolls " + _myMouseFocused + " by [" + theDeltaX + "," + theDeltaY + "]");
            var myEvent = new spark.MouseEvent(
                spark.MouseEvent.SCROLL,
                _myMousePosition.x, _myMousePosition.y,
                theDeltaX, theDeltaY, null, myButtonStates
            );
            _myMouseFocused.dispatchEvent(myEvent);
        }
    };

    // Will be called on a joystick axis event
    Base.onAxis = Public.onAxis;
    Public.onAxis = function (device, axis, value) {
        Base.onAxis(device, axis, value);
    };

    // Will be called on a joystick button event
    Base.onButton = Public.onButton;
    Public.onButton = function (theDevice, theButton, theState) {
        Base.onButton(theDevice, theButton, theState);
    };

    // Will be called on a window reshape event
    Base.onResize = Public.onResize;
    Public.onResize = function (theNewWidth, theNewHeight) {
        Base.onResize(theNewWidth, theNewHeight);
    };

    // Will be called before exit
    Base.onExit = Public.onExit;
    Public.onExit = function () {
        Base.onExit();
    };

    // Handle DSA events
    Public.onTouch = function (theEventName, theId, theBitMask, theGridSizeX, theGridSizeY, theCount) {
        Logger.trace("onTouch " + theEventName +
                     ", " + theId + ", " + theBitMask + ", " + theGridSizeX +
                     ", " + theGridSizeY + ", " + theCount);
        var myEvent = new spark.DSAEvent(spark.DSAEvent.TOUCH, theEventName, theId, theBitMask,
                                         new Vector2f(theGridSizeX, theGridSizeY), theCount);
        Public.dispatchEvent(myEvent);
    };
    
    Public.onASSEvent = handleMultitouchEvent;
    Public.onTuioEvent = handleMultitouchEvent;
    
    // Will be called on a gesture event
    Public.onGesture = function (theGesture) {
        
        var mySparkConformedCursorId = getSparkConformedCursorId(theGesture, theGesture.cursorid);
        var myPosition = getMultitouchCursorPosition(theGesture);
        
        // picking with considering cursor grabbing
        var myCursor = getMultitouchCursor(mySparkConformedCursorId);
        var myWidget = getWidgetForMultitouchCursor(myCursor, myPosition);
        
        var myScale = new Vector3f(1, 1, 1);
        if (theGesture.baseeventtype == TUIO_BASE_EVENT) {
            myScale = new Vector3f((theGesture.toucharea.x > 0) ? theGesture.toucharea.x : Public.width,
                                   (theGesture.toucharea.y > 0) ? theGesture.toucharea.y : Public.height, 1);
        }
        var myCursorPartner = null;
        var myCenterPoint = null;
        // Do some multicursor gesture specific things (cursor_pair_start, zoom, rotate, cursor_pair_finish)
        if (theGesture.type == "cursor_pair_start" || theGesture.type == "zoom" || theGesture.type == "cursor_pair_finish" || theGesture.type == "rotate") {
            if (theGesture.type != "cursor_pair_finish") {
                myCenterPoint = new Vector3f(theGesture.centerpoint.x, theGesture.centerpoint.y, 0);
                myCenterPoint.mult(myScale);
            }
            var mySparkConformedCursorPartnerId = getSparkConformedCursorId(theGesture, theGesture.cursorpartnerid);
            myCursorPartner = getMultitouchCursor(mySparkConformedCursorPartnerId);
        }
        
        switch (theGesture.type) {
        case "tap": 
            var myTapEvent = new spark.TapGestureEvent(spark.GestureEvent.TAP, theGesture.baseeventtype, myCursor, myPosition);
            myWidget.dispatchEvent(myTapEvent);
            break;
        case "wipe":
            var myVelocity = magnitude(myScale) * theGesture.velocity;
            var myWipeEvent = new spark.WipeGestureEvent(spark.GestureEvent.WIPE, theGesture.baseeventtype, myCursor, theGesture.direction, myVelocity);
            myWidget.dispatchEvent(myWipeEvent);
            break;
        case "cursor_pair_start":
            var myCursorPairStartEvent = new spark.MultiCursorGestureEvent(spark.GestureEvent.CURSOR_PAIR_START, theGesture.baseeventtype, myCursor, myCursorPartner, myCenterPoint, theGesture.distance);
            myWidget.dispatchEvent(myCursorPairStartEvent);
            break;
        case "zoom":
            var myDistance = theGesture.distance;
            var myLastDistance = theGesture.lastdistance;
            var myInitialDistance = theGesture.initialdistance;
            var myZoomFactor = theGesture.zoomfactor;
            var myZoomEvent = new spark.ZoomGestureEvent(spark.GestureEvent.ZOOM, theGesture.baseeventtype, myCursor, myCursorPartner, myCenterPoint, myDistance, myLastDistance, myInitialDistance, myZoomFactor);
            myWidget.dispatchEvent(myZoomEvent);
            break;
        case "rotate":
            var myRotateEvent = new spark.RotateGestureEvent(spark.GestureEvent.ROTATE, theGesture.baseeventtype, myCursor, myCursorPartner, myCenterPoint, theGesture.distance, theGesture.angle);
            myWidget.dispatchEvent(myRotateEvent);
            break;
        case "cursor_pair_finish":
            var myCursorPairFinishEvent = new spark.MultiCursorGestureEvent(spark.GestureEvent.CURSOR_PAIR_FINISH, theGesture.baseeventtype, myCursor, myCursorPartner);
            myWidget.dispatchEvent(myCursorPairFinishEvent);
            break;
        default:
            Logger.info("Unknown gesture : " + theGesture);
        }
    };
};
