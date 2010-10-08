//=============================================================================
// Copyright (C) 2009, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

//howto use
//hold ctrl, click target, move with mouse or arrows or shift-arrows
//+ moves target to parent
//a toggles additional features of target
//s change size of widget
//1 moves down in z direction
//2 moves up in z direction

//classes using Layouter can implement hooks
// * layouterUpdatePosition
// * writebackPosition
// * layouterToggleWidget - can be used to change widget style e.g.

spark.Layouter = spark.ComponentClass("Layouter");
spark.Layouter.Constructor = function(Protected) {

    var Base = {};
    var Public = this;
    this.Inherit(spark.Component);
    
    var _myState       = IDLE;
    var _myOldPos      = null;
    var _myWidget      = null;
    var _myShiftFlag   = false;
    var _myIsCtrlPressed = false;
    var _myIsSPressed = false;   //change size 
    var _myIsYPressed = false;   //change z position
    var _myStage       = null;
    var _myListeners = null;
    var _myCurrentSparkNode = null;
    var _myCurrentSparkFile = null;
    var _mySparkNodes = [];
    var _mySparkFiles = [];
    var _myVisualMode = 2; // box with mesurement
    const DEBUG_COLOR = new Vector4f(1,0,0,1);
    
    const IDLE = 0;
    const ACTIVE = 1;
    
    Base.realize = Public.realize;
    Public.realize = function () {
        Base.realize();
        _myStage = findStage();
        _myStage.focusKeyboard(_myStage);
        
        Base.onKey = _myStage.onKey;
        _myStage.onKey = function(theKey, theKeyState, theX, theY,
                             theShiftFlag, theControlFlag, theAltFlag) {
            if (!theControlFlag) {
                Base.onKey(theKey, theKeyState, theX, theY, theShiftFlag, theControlFlag, theAltFlag);
            }
            Logger.info("onKey "+ theKey+" "+_myIsYPressed+" "+_myIsSPressed+" "+_myShiftFlag);
            if(theKeyState) {
                if(theKey.search(/ctrl/) != -1) {
                    _myIsCtrlPressed = true;
                    activate();
                } else if (theKey.search(/shift/) != -1) {
                    _myShiftFlag = true;
                } else if (theKey == "s") {
                    _myIsSPressed = true;
                } else if (theKey == "y") {
                    _myIsYPressed = true;
                } else if ((theKey == "left"
                        || theKey == "right"
                        || theKey == "up"
                        || theKey == "down") && Public.active) {
                    correctPositionAndSize(theKey);
                } else if (((theKey == "+")||(theKey == "]")) && Public.active && _myWidget && _myWidget.parent) { 
                    Public.target = _myWidget.parent;
                    print("moved targed to parent: ", Public.target);
                } else if ((theKey == "a") && Public.active && _myWidget && "layouterToggleWidget" in _myWidget) {
                    _myWidget.layouterToggleWidget();
                } else if (theKey == "x") {
                    _myVisualMode = (_myVisualMode + 1)%3;
                }
            } else {
                if(theKey.search(/ctrl/) != -1) {
                    _myIsCtrlPressed = false;
                    deactivate();
                    stop();   
                } else if (theKey.search(/shift/) != -1) {
                    _myShiftFlag = false;
                } else if (theKey == "s") {
                    _myIsSPressed = false;
                } else if (theKey == "y") {
                    _myIsYPressed = false;
                }
            }
        }
        _myStage.addEventListener(spark.StageEvent.POST_RENDER, onPostRender);
    };
    
    function onPostRender() {
        if (_myIsCtrlPressed && Public.active && _myWidget && _myVisualMode > 0) {
            var myMatrix = new Matrix4f();
            var myBox = _myWidget.sceneNode.boundingbox;
            window.getRenderer().draw(myBox, DEBUG_COLOR, myMatrix, 2);
            if (_myVisualMode > 1) {
                window.getRenderer().draw(new LineSegment(new Point3f(0,myBox.min.y,myBox.min.z),new Point3f(window.width,myBox.min.y,myBox.min.z) ), DEBUG_COLOR, myMatrix, 1);
                window.getRenderer().draw(new LineSegment(new Point3f(myBox.min.x, 0, myBox.min.z),new Point3f(myBox.min.x,window.height,myBox.min.z) ), DEBUG_COLOR, myMatrix, 1);
                window.setTextColor([1,0,0,1]);
                var myViewport = _myStage.getViewportAtWindowCoordinates(0, 0); // get viewport containing upper left pixel            
                var myDifference = difference(myBox.min, new Vector3f(0,0,myBox.min.z));
                var myDifferenceTopLeft = difference(myBox.min, new Vector3f(0,window.height,myBox.min.z));
                var myDebugPosition = [];
                window.renderText([clamp(myBox.min.x-80,0,window.width), clamp(window.height - (myBox.min.y-20), 0, window.height)], "[" + myDifference.x  + "," + myDifference.y + "]", "Screen13", myViewport );
                window.renderText([clamp(myBox.min.x-80, 0,window.width), clamp(window.height - (myBox.min.y+20), 0,window.height)], "[" + myDifferenceTopLeft.x  + "," + Math.abs(myDifferenceTopLeft.y) + "]", "Screen13", myViewport );
            }
        }
        
    }
    // poor man's Public.stage getter
    function findStage() {
        for (object in spark.ourComponentsByNameMap) {
            if("Stage" in spark.ourComponentsByNameMap[object]._classes_) {
                return spark.ourComponentsByNameMap[object];
            }    
        }
    }

    Public.active getter = function () {
        return (_myState == ACTIVE);
    };

    Public.target setter = function (theWidget) {
        print("target widget", theWidget)
        _myWidget = theWidget;
        _myState = ACTIVE;
        _myOldPos = null;
        if (!(_myWidget.name in _mySparkFiles)) {
            var myFile = findSparkFile(_myWidget);
            _mySparkFiles[_myWidget.name] = myFile;
        }
        if (myFile) {
            if (!(myFile in _mySparkNodes)) {
                var myFileWithPath = searchFile(myFile);
                var myNode = new Node();
                myNode.parseFile(myFileWithPath);
                _myCurrentSparkNode = myNode;
                _mySparkNodes[myFile] = myNode;
            } else {
                _myCurrentSparkNode = _mySparkNodes[myFile];
            }
            _myCurrentSparkFile = myFile;
        }
    };

    function findSparkFile(theWidget) {
        if (!theWidget) {
            return "";
        }
        if (theWidget._sparkFile_) {
            return theWidget._sparkFile_;
        }
        return findSparkFile(theWidget.parent);
    }

    function updatePosition(theX, theY, theZ) {
        if(!_myOldPos) {
            _myOldPos = new Vector3f(theX, window.height - theY, theZ);
            return;
        }
        if ("layouterUpdatePosition" in _myWidget) {
            _myWidget.layouterUpdatePosition();
        }
        var myRotation = _myWidget.sceneNode.globalmatrix.getRotation();
        var myWidgetRotation = new Vector3f(radFromDeg(_myWidget.rotation.x), radFromDeg(_myWidget.rotation.y), radFromDeg(_myWidget.rotation.z));
        myRotation = difference(myRotation, myWidgetRotation);
        var myXOffset = theX - _myOldPos.x;
        var myYOffset = (window.height - theY) - _myOldPos.y;
        var myZOffset = theZ - _myOldPos.z;
        var myMatrix = new Matrix4f();
        myMatrix.makeTranslating(new Vector3f(myXOffset, myYOffset, myZOffset));
        myMatrix.rotateZ(myRotation.z);
        var myTranslation = myMatrix.getTranslation();
        _myWidget.x += myTranslation.x;
        _myWidget.y += myTranslation.y;
        _myWidget.z += myTranslation.z;
        print(_myWidget.name + " moved to x: " + _myWidget.x + " y: " + _myWidget.y + " z: " +_myWidget.z);
        _myOldPos = new Vector3f(theX, window.height - theY, theZ);
    }
    
    function activate () {
        print("activate")
        _myStage.addEventListener(spark.CursorEvent.APPEAR, onMouseDown , true);
        _myStage.addEventListener(spark.CursorEvent.MOVE, onMouseMove, true);
    }
    
    function deactivate() {
        print("deactivate")
        _myStage.removeEventListener(spark.CursorEvent.APPEAR, onMouseDown, true);
        _myStage.removeEventListener(spark.CursorEvent.MOVE, onMouseMove, true);
        if (_myWidget) {
            for (var i = 0; i < _myListeners.length; ++i) {
                _myWidget.addEventListener(spark.CursorEvent.APPEAR, _myListeners[i].listener);
            }
        }
    }

    function onMouseDown(theEvent) {
        if (!_myWidget) {
            print("onMouseDown", theEvent.target)
            Public.target = theEvent.target;
            _myListeners = clone(theEvent.target.getEventListeners(spark.CursorEvent.APPEAR));
        }
        for (var i = 0; i < _myListeners.length; ++i) {
            theEvent.target.removeEventListener(spark.CursorEvent.APPEAR, _myListeners[i].listener, _myListeners[i].useCapture);
        }
        updatePosition(theEvent.stageX, theEvent.stageY, 0);
    }

    function onMouseMove(theEvent) {
        if (_myIsCtrlPressed && Public.active) {
            updatePosition(theEvent.stageX, theEvent.stageY, 0);
        }
    }

    function correctPositionAndSize(theKey) {
        var myNewPos = new Vector3f(0, 0, 0);
        var myDist = 1;
        if (_myShiftFlag) {
            myDist = 5;
        }
        if (_myIsYPressed) {  // z-pos manipulation
            switch(theKey) {
                case "up":
                    myNewPos = new Vector3f(_myOldPos.x, _myOldPos.y, _myOldPos.z + myDist);
                    break;  
                case "down":
                    myNewPos = new Vector3f(_myOldPos.x, _myOldPos.y, _myOldPos.z - myDist);
                    break;              
                default:
                    break;
            }
            updatePosition(myNewPos.x, window.height - myNewPos.y, myNewPos.z);
        } else if (_myIsSPressed) { // size manipulation
            switch(theKey) {
                case "left":
                    if (_myWidget.width) {
                        _myWidget.width -= myDist;
                    } 
                    break;
                case "right":
                    if (_myWidget.width) {
                        _myWidget.width += myDist;
                    } 
                    break;
                case "up":
                    if (_myWidget.height) {
                        _myWidget.height += myDist;
                    } 
                    break;  
                case "down":
                    if (_myWidget.height) {
                        _myWidget.height -= myDist;
                    } 
                    break;              
                default:
                    break;
            }
        } else { 
            switch(theKey) {
                case "left":
                    myNewPos = new Vector3f(_myOldPos.x-myDist, _myOldPos.y, _myOldPos.z);
                    break;
                case "right":
                    myNewPos = new Vector3f(_myOldPos.x+myDist, _myOldPos.y, _myOldPos.z);
                    break;
                case "up":
                    myNewPos = new Vector3f(_myOldPos.x, _myOldPos.y+myDist, _myOldPos.z);
                    break;  
                case "down":
                    myNewPos = new Vector3f(_myOldPos.x, _myOldPos.y-myDist, _myOldPos.z);
                    break;              
                default:
                    break;
            }
            updatePosition(myNewPos.x, window.height - myNewPos.y, myNewPos.z);
        }
    }

    function stop() {
        print("stop widget:"+_myWidget);
        _myState = IDLE;
        _myOldPos = null;
        if (_myWidget && _myCurrentSparkNode) {
            if ("writebackPosition" in _myWidget) {
                _myWidget.writebackPosition(_myCurrentSparkNode, _myCurrentSparkFile);
            } else {
                var myNode = findWritebackNode(_myWidget);
                if (myNode) {
                    myNode.x = Math.round(_myWidget.x);
                    myNode.y = Math.round(_myWidget.y);
                    myNode.z = Math.round(_myWidget.z);
                    if (_myWidget.width) {
                        myNode.width = Math.round(_myWidget.width);
                    }
                    if (_myWidget.height) {
                        myNode.height = Math.round(_myWidget.height);
                    }
                    _myCurrentSparkNode.saveFile(_myCurrentSparkFile);
                    print("write to file ", _myCurrentSparkFile);
                }
            }
        }
        _myWidget = null;
    }

    function findWritebackNode(theWidget) {
        var myNodes = _myCurrentSparkNode.findAll(".//*[@name='" + _myWidget.name + "']");
        for (var i = 0, l = myNodes.length; i < l; ++i) {
            var myWidget = theWidget;
            var myNode = myNodes[i];
            while(myWidget && myWidget.parent && myNode.parentNode && !("Stage" in myWidget._classes_)) {
                if (myWidget.parent.name && "name" in myNode.parentNode && myWidget.parent.name !== myNode.parentNode.name) 
                {
                    myWidget = null;
                    break;
                }
                myWidget = myWidget.parent;
                myNode = myNode.parentNode;
            }
            if (myWidget) {
                return myNodes[i];
            }
        }
        return null;
    }

}

