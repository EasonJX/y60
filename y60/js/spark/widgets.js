/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2009, ART+COM AG Berlin, Germany <www.artcom.de>
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
// Description: Standard widgets
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

/**
 * Widgets wrap Y60 scene objects.
 * 
 * This is where positioning, orientation, scaling,
 * visibility and sensibility go.
 */
spark.Widget = spark.AbstractClass("Widget");

spark.Widget.Constructor = function(Protected) {
    var Base = {};
    var Public = this;

    this.Inherit(spark.Container);
    this.Inherit(spark.EventDispatcher);

    var _mySceneNode   = null;
    
    // XXX: try to get rid of this.
    Public.sceneNode getter = function() {
        return _mySceneNode;
    };
    
    // XXX: try to get rid of this.
    Public.innerSceneNode getter = function() {
        return _mySceneNode;
    };

    // XXX: function for getting screen-aligned bounds
    Public.worldposition getter = function() {
        return _mySceneNode.globalmatrix.getTranslation();
    };

    // XXX: function for getting bounds
    Public.size getter = function() {
        var myBoundingBox = _mySceneNode.boundingbox;
        var myWidth = 0;
        if(myBoundingBox == "[]"){
            Logger.warning("BoundingBox not initialized - size getter not yet implemented");
            return new Vector2f(0,0);
        } else {
            return myBoundingBox.size;
        }
    };

    Public.width getter = function(){
        return Public.size.x;
    };
    
    Public.height getter = function(){
        return Public.size.y;
    };


    // STAGE

    Public.stage getter = function() {
        var myCurrent = Public;
        while(myCurrent) {
            if("Stage" in myCurrent._classes_) {
                return myCurrent;
            }
            myCurrent = myCurrent.parent;
        }
        Logger.fatal("Widget " + Public.name + " is not the child of a valid stage.");
    };

    
    // STAGE EVENTS

    Base.addEventListener = Public.addEventListener;
    Public.addEventListener = function(theType, theListener, theUseCapture) {
        if(!("Stage" in Public._classes_)) {
            switch(theType) {
            case spark.StageEvent.FRAME:
            case spark.StageEvent.PRE_RENDER:
            case spark.StageEvent.POST_RENDER:
                if(theUseCapture) {
                    Logger.fatal("Capture of stage events is forbidden.");
                }
                Public.stage.addEventListener(
                    theType,
                    function(theEvent) {
                        Public.dispatchEvent(theEvent);
                    },
                    false
                );
                break;
            }
        }
        Base.addEventListener(theType, theListener, theUseCapture);
    };

    Base.removeEventListener = Public.removeEventListener;
    Public.removeEventListener = function(theType, theListener, theUseCapture) {
        switch(theType) {
        case spark.StageEvent.FRAME:
        case spark.StageEvent.PRE_RENDER:
        case spark.StageEvent.POST_RENDER:
            Logger.fatal("Removal of stage event listeners is not supported");
            break;
        default:
            Base.removeEventListener(theType, theListener, theUseCapture);
            break;
        }
    };
    
    // ALPHA

    var _myAlpha = 1.0;

    Public.Getter("alpha", function() {
        return _myAlpha;
    });

    Public.Setter("alpha", function(theValue) {
        _myAlpha = theValue;
        Public.propagateAlpha();
    });


    var _myActualAlpha = 1.0;

    Public.Getter("actualAlpha", function() {
        return _myActualAlpha;
    });


    Public.Getter("parentAlpha", function() {
        var myParentAlpha = 1.0;
        if((Public.parent) && ("actualAlpha" in Public.parent)) {
            myParentAlpha = Public.parent.actualAlpha;
        }
        return myParentAlpha;
    });


    Public.propagateAlpha = function() {
        _myActualAlpha = Public.parentAlpha * _myAlpha;

        for(var i = 0; i < Public.children.length; i++) {
            var myChild = Public.children[i];
            if("propagateAlpha" in myChild) {
                myChild.propagateAlpha();
            }
        }
    };
    
    
    // VISIBILITY AND SENSIBLITY

    Public.Property("visible", Boolean, true, applyVisibleAndSensible);

    Public.Property("sensible", Boolean, true, applyVisibleAndSensible);

    function applyVisibleAndSensible() {
        if(_mySceneNode) {
            _mySceneNode.insensible = !(Public.visible && Public.sensible);
            _mySceneNode.visible    = Public.visible;
        }
    }

    // POSITION

    Public.Getter("position", function() {
        return new Vector3f(Public.x, Public.y, Public.z);
    });

    Public.Setter("position", function(theValue) {
        Public.x = theValue.x;
        Public.y = theValue.y;
        Public.z = theValue.z;
    });

    this.Property("x", Number, 0.0, applyPosition);
    this.Property("y", Number, 0.0, applyPosition);
    this.Property("z", Number, 0.0, applyPosition);

    function applyPosition() {
        if(_mySceneNode) {
            _mySceneNode.position = Public.position;
        }
    };

    // SCALE
    
    Public.Getter("scale", function() {
        return new Vector3f(Public.scaleX, Public.scaleY, Public.scaleZ);
    });

    Public.Setter("scale", function(theValue) {
        Public.scaleX = theValue.x;
        Public.scaleY = theValue.y;
        Public.scaleZ = theValue.z;
    });

    this.Property("scaleX", Number, 1.0, applyScale);
    this.Property("scaleY", Number, 1.0, applyScale);
    this.Property("scaleZ", Number, 1.0, applyScale);

    function applyScale() {
        if(_mySceneNode) {
            _mySceneNode.scale = Public.scale;
        }
    };


    // PIVOT

    Public.Getter("pivot", function() {
        return new Vector3f(Public.pivotX, Public.pivotY, Public.pivotZ);
    });

    Public.Setter("pivot", function(theValue) {
        Public.pivotX = theValue.x;
        Public.pivotY = theValue.y;
        Public.pivotZ = theValue.z;
    });

    this.Property("pivotX", Number, 0.0, applyPivot);
    this.Property("pivotY", Number, 0.0, applyPivot);
    this.Property("pivotZ", Number, 0.0, applyPivot);

    function applyPivot() {
        if(_mySceneNode) {
            _mySceneNode.pivot = Public.pivot;
        }
    };


    // ORIGIN
    // XXX: origins must be set up before realization

    Public.Getter("origin", function() {
        return new Vector3f(Public.originX, Public.originY, Public.originZ);
    });

    Public.Setter("origin", function(theValue) {
        Public.originX = theValue.x;
        Public.originY = theValue.y;
        Public.originZ = theValue.z;
    });

    this.Property("originX", Number, 0.0, applyOrigin);
    this.Property("originY", Number, 0.0, applyOrigin);
    this.Property("originZ", Number, 0.0, applyOrigin);

    function applyOrigin() {
    };


    // ROTATION
    
    Public.Getter("rotation", function() {
        return new Vector3f(Public.rotationX, Public.rotationY, Public.rotationZ);
    });

    Public.Setter("rotation", function(theValue) {
        Public.rotationX = theValue.x;
        Public.rotationY = theValue.y;
        Public.rotationZ = theValue.z;
    });

    this.Property("rotationX", Number, 0.0, applyRotation);
    this.Property("rotationY", Number, 0.0, applyRotation);
    this.Property("rotationZ", Number, 0.0, applyRotation);

    function applyRotation() {
        if(_mySceneNode) {
            var myRotation = new Vector3f(radFromDeg(Public.rotationX),
                                          radFromDeg(Public.rotationY),
                                          radFromDeg(Public.rotationZ));
            
            var myQuaternion = Quaternionf.createFromEuler(myRotation);
            
            _mySceneNode.orientation = myQuaternion;
        }
    };

    // INTERNATIONALISATION HOOKS

    var _myI18nContext = null;

    Public.i18nContext getter = function() {
        if(_myI18nContext) {
            return _myI18nContext;
        } else {
            if(Public.parent) {
                return Public.parent.i18nContext;
            } else {
                return null;
            }
        }
    };

    Public.i18nContext setter = function(theValue) {
        _myI18nContext = theValue; // XXX: re-trigger i18n events? how?
    };

    Public.i18nContexts getter = function() {
        var myContexts = [];
        var myCurrent = Public;
        while(myCurrent) {
            if(myContexts.length > 0) {
                var myContext = myCurrent.i18nContext;
                if(myContexts[myContexts.length - 1] != myContext) {
                    myContexts.push(myCurrent.i18nContext);
                }
            } else {
                myContexts.push(myCurrent.i18nContext);
            }
            myCurrent = myCurrent.parent;
        }
        return myContexts;
    };

    Public.getI18nItemByName = function(theName) {
        var myContexts = Public.i18nContexts;
        for(var i = 0; i < myContexts.length; i++) {
            var myContext = myContexts[i];
            var myItem = myContext.getChildByName(theName);
            if(myItem) {
                return myItem;
            }
        }
        return null;
    };

    // ANIMATION HELPERS

    Public.animateProperty = function(theDuration, theEasing, theProperty, theStart, theEnd) {
        return new GUI.PropertyAnimation(theDuration, theEasing, Public, theProperty, theStart, theEnd);
    };

    Public.animateFade = function(theDuration, theEnd, theEasing) {
        if(!theDuration) {
            theDuration = 250.0;
        }
        if(!theEnd) {
        	theEnd = 1.0;
        }
        if(!theEasing) {
            theEasing = null;
        }
        return Public.animateProperty(theDuration, theEasing, "alpha", Public.alpha, theEnd);
    };
    
    Public.animateFadeIn = function(theDuration, theEasing) {
        if(!theDuration) {
            theDuration = 250.0;
        }
        if(!theEasing) {
            theEasing = null;
        }
        return Public.animateProperty(theDuration, theEasing, "alpha", 0.0, 1.0);
    };

    Public.animateFadeOut = function(theDuration, theEasing) {
        if(!theDuration) {
            theDuration = 250.0;
        }
        if(!theEasing) {
            theEasing = null;
        }
        return Public.animateProperty(theDuration, theEasing, "alpha", 1.0, 0.0);
    };



    Base.realize = Public.realize;
    Public.realize = function(theSceneNode) {
        _mySceneNode = theSceneNode;

        _mySceneNode.sticky = true;

        spark.sceneNodeMap[_mySceneNode.id] = Public;

        Base.realize();

        applyVisibleAndSensible();
        applyPosition();
        applyScale();
        applyRotation();
        applyPivot();

        Public.propagateAlpha();
    };

    Base.postRealize = Public.postRealize;
    Public.postRealize = function() {
        var myContextName = Protected.getString("i18nContext", "");
        if(myContextName != "") {
            _myI18nContext = Public.getChildByName(myContextName);
        }

        Base.postRealize();
    };
    
}
