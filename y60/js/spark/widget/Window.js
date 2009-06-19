/**
 * Simple wrapper for the Y60 scene viewer.
 */

use("SceneViewer.js"); // XXX: bad place for this

spark.Window = spark.ComponentClass("Window");

spark.Window.Constructor = function(Protected) {
    var Public = this;
    var Base = {};

    this.Inherit(spark.Stage);

    SceneViewer.prototype.Constructor(this, []);

    Public.title getter = function()  {
        return window.title;
    };
    
    Public.title setter = function(theTitle) {
        window.title = theTitle;
    };
    
    Base.realize = Public.realize;
    Public.realize = function() {
        window = new RenderWindow();
        
        window.position = [
            Protected.getNumber("positionX", 0),
            Protected.getNumber("positionY", 0)
        ];
        
        window.decorations = Protected.getBoolean("decorations", true);
        
        Public.setup(Protected.getNumber("width", 640),
                     Protected.getNumber("height", 480),
                     Protected.getBoolean("fullscreen", false),
                     Protected.getString("title", "SPARK Application"));

        Public.setMover(null);
        
        window.showMouseCursor = Protected.getBoolean("mouseCursor", true);
        window.swapInterval = Protected.getNumber("swapInterval", 1);
        
        spark.setupCameraOrtho(window.scene.dom.find(".//camera"), window.width, window.height);
        
        Base.realize(window.scene.world);
    };
    
    // XXX: Override size, width and height properties inherited via Stage->Widget
    // They do return a boundingbox size which, from class Windows standpoint, is just
    // crap.
    Public.size getter = function() {
        return new Vector2i(window.width, window.height);
    }
    Public.width getter = function() {
        return window.width;
    }
    Public.height getter = function() {
        return window.height;
    }

    const PICK_RADIUS = 1;
    
    Public.pickWidget = function(theX, theY) {
        var myBody = Public.picking.pickBodyBySweepingSphereFromBodies(theX, theY, PICK_RADIUS, Public.sceneNode);
        if(myBody) {
            var myBodyId = myBody.id;
            if(myBodyId in spark.sceneNodeMap) {
                var myWidget = spark.sceneNodeMap[myBodyId];
                return myWidget;
            }
        }
        return null;
    };
    
    
    var _myMousePosition = new Vector2f();
    
    Public.mousePosition getter = function() {
    };
    
    Protected.updateMousePosition = function(theX, theY) {
        _myMousePosition.x = theX;
        _myMousePosition.y = theY;
    };
    
    
    var _myMouseFocused = null;
    
    Public.mouseFocused getter = function() {
        return _myMouseFocused;
    };
    
    
    var _myKeyboardFocused = null;
    
    Public.keyboardFocused getter = function() {
        return _myKeyboardFocused;
    };
    
    Public.focusKeyboard = function(theWidget) {
        _myKeyboardFocused = theWidget;
    };
    
    
    //////////////////////////////////////////////////////////////////////
    // Callbacks
    //////////////////////////////////////////////////////////////////////

    //  Will be called first in renderloop, has the time since application start
    Base.onFrame = Public.onFrame;
    Public.onFrame = function(theTime, theDeltaT) {
        Base.onFrame(theTime, theDeltaT);
        if(Public.hasEventListener(spark.StageEvent.FRAME)) {
            var myEvent = new spark.StageEvent(spark.StageEvent.FRAME, Public, theTime, theDeltaT);
            Public.dispatchEvent(myEvent);
        }
    };

    // Will be called before rendering the frame
    Base.onPreRender = Public.onPreRender;
    Public.onPreRender = function() {
        Base.onPreRender();
        var myEvent = new spark.StageEvent(spark.StageEvent.PRE_RENDER, Public);
        Public.dispatchEvent(myEvent);
    };

    // Will be called after rendering the frame, but before swap buffer
    Base.onPostRender = Public.onPostRender;
    Public.onPostRender = function() {
        Base.onPostRender();
        var myEvent = new spark.StageEvent(spark.StageEvent.POST_RENDER, Public);
        Public.dispatchEvent(myEvent);
    };

    // Will be called on a mouse move
    Base.onMouseMotion = Public.onMouseMotion;
    Public.onMouseMotion = function(theX, theY) {
        Base.onMouseMotion(theX, theY);
        
        Protected.updateMousePosition(theX, theY);
        
        var myWidget = Public.pickWidget(theX, theY);
        
        if(!myWidget) {
            myWidget = Public;
        }
        
        if(myWidget != _myMouseFocused) {
            Logger.debug("Mouse focuses " + myWidget
                         + (_myMouseFocused ? ", leaving " + _myMouseFocused : ""));
            
            if(_myMouseFocused) {
                var myLeaveEvent = new spark.MouseEvent(spark.MouseEvent.LEAVE, theX, theY);
                _myMouseFocused.dispatchEvent(myLeaveEvent);
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
    Public.onMouseButton = function(theButton, theState, theX, theY) {
        Base.onMouseButton(theButton, theState, theX, theY);
        
        Protected.updateMousePosition(theX, theY);

        var myWidget = Public.pickWidget(theX, theY);
        
        if(myWidget) {
            if(theState) {
                // XXX: click should be more well-defined and button-up-based.
                Logger.debug("Mouse clicks " + myWidget);
                var myClickEvent = new spark.MouseEvent(spark.MouseEvent.CLICK, theX, theY);
                myWidget.dispatchEvent(myClickEvent);
                
                Logger.debug("Mouse button down on " + myWidget);
                var myDownEvent = new spark.MouseEvent(spark.MouseEvent.BUTTON_DOWN, theX, theY);
                myWidget.dispatchEvent(myDownEvent);
            } else {
                Logger.debug("Mouse button up on " + myWidget);
                var myUpEvent = new spark.MouseEvent(spark.MouseEvent.BUTTON_UP, theX, theY);
                myWidget.dispatchEvent(myUpEvent);                
            }
        }
    };

    // Will be called on a keyboard event
    Base.onKey = Public.onKey;
    Public.onKey = function(theKey, theKeyState, theX, theY,
                         theShiftFlag, theControlFlag, theAltFlag) {
        Base.onKey(theKey, theKeyState, theX, theY, theShiftFlag, theControlFlag, theAltFlag);
        
        if(_myKeyboardFocused) {
            var myModifiers =
                (theShiftFlag ? spark.Keyboard.SHIFT : 0)
                | (theControlFlag ? spark.Keyboard.CTRL : 0)
                | (theAltFlag ? spark.Keyboard.ALT : 0);
            var myType = theKeyState ? spark.KeyboardEvent.KEY_DOWN : spark.KeyboardEvent.KEY_UP;
            var myEvent = new spark.KeyboardEvent(myType, theKey, myModifiers);
            Logger.debug("Key " + myEvent.keyString + " " + (theKeyState ? "down" : "up") + " on " + _myKeyboardFocused);
            _myKeyboardFocused.dispatchEvent(myEvent);
        }
    };

    // Will be called on a mouse wheel event
    Base.onMouseWheel = Public.onMouseWheel;
    Public.onMouseWheel = function(theDeltaX, theDeltaY) {
        Base.onMouseWheel(theDeltaX, theDeltaY);
        
        if(_myMouseFocused) {
            Logger.debug("Mouse scrolls " + _myMouseFocused + " by [" + theDeltaX + "," + theDeltaY + "]");
            var myEvent = new spark.MouseEvent(
                spark.MouseEvent.SCROLL,
                _myMousePosition.x, _myMousePosition.y,
                theDeltaX, theDeltaY
            );
            _myMouseFocused.dispatchEvent(myEvent);
        }
    };

    // Will be called on a joystick axis event
    Base.onAxis = Public.onAxis;
    Public.onAxis = function(device, axis, value) {
        Base.onAxis(device, axis, value);
    };

    // Will be called on a joystick button event
    Base.onButton = Public.onButton;
    Public.onButton = function(theDevice, theButton, theState) {
        Base.onButton(theDevice, theButton, theState);
    };

    // Will be called on a window reshape event
    Base.onResize = Public.onResize;
    Public.onResize = function(theNewWidth, theNewHeight) {
        Base.onResize(theNewWidth, theNewHeight);
    };

    // Will be called before exit
    Base.onExit = Public.onExit;
    Public.onExit = function() {
        Base.onExit();
    };
    
    
    var _myASSCursors = {};

    Public.onASSEvent = function(theEvent) {
        spark.proximatrix.onASSEvent(theEvent);
        
        var myId = theEvent.id;
        
        switch(theEvent.type) {
        case "configure":
            break;
            
        case "add":
        case "move":
            var myCursor;
            if(myId in _myASSCursors) {
                myCursor = _myASSCursors[myId];
            } else {
                Logger.debug("Cursor " + myId + " added");
                myCursor = new spark.Cursor(myId);
                _myASSCursors[myId] = myCursor;
            }

            if(theEvent.type == "add") {
                myCursor.activate();
            }
            
            var myFocused = myCursor.focused;
            
            var myPick = Public.pickWidget(theEvent.position3D.x, theEvent.position3D.y);
            if(!myPick) {
                myPick = Public;
            }
            
            myCursor.update(theEvent, myPick);
            
            if(myPick != myFocused) {
                Logger.debug("Cursor " + myId + " focuses " + myPick
                             + (myFocused ? ", leaving " + myFocused : ""));
                
                if(myFocused) {
                    var myLeave = new spark.CursorEvent(spark.CursorEvent.LEAVE, myCursor);
                    myFocused.dispatchEvent(myLeave);
                }
                
                var myEnter = new spark.CursorEvent(spark.CursorEvent.ENTER, myCursor);
                myPick.dispatchEvent(myEnter);
            }
            
            if(theEvent.type == "move") {
                Logger.debug("Cursor " + myId + " moves to " + theEvent.position3D + " over " + myPick);
                var myMove = new spark.CursorEvent(spark.CursorEvent.MOVE, myCursor);
                myPick.dispatchEvent(myMove);
            }
            
            break;
            
        case "remove":
            if(myId in _myASSCursors) {
                Logger.debug("Cursor " + myId + " removed");
                
                var myCursor = _myASSCursors[myId];
                
                var myFocused = myCursor.focused;
                
                myCursor.update(theEvent, myFocused);
                
                if(myFocused) {
                    Logger.debug("Cursor " + myId + " leaves " + myFocused);
                    var myLeave = new spark.CursorEvent(spark.CursorEvent.LEAVE, myCursor);
                    myFocused.dispatchEvent(myLeave);
                }
                
                myCursor.deactivate();
            }
            break;
        }
    };
    
};
