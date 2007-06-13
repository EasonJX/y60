//=============================================================================
// Copyright (C) 2003-2004, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: ClassicTrackballMover.js,v $
//   $Author: christian $
//   $Revision: 1.7 $
//   $Date: 2005/04/20 16:49:06 $
//
//=============================================================================

// Possible improvements:
// - Use swept sphere for trackball center detection in stead of stick
// - Adapt fly/zoom/pan-speed to height above ground

use("MoverBase.js");
use("picking_functions.js");
use("intersection_functions.js");

function ClassicTrackballMover(theViewport) {
    this.Constructor(this, theViewport);
}

function CenteredTrackballMover(theViewport) {
    ClassicTrackballMover.prototype.Constructor(this, theViewport, true);
    this.name = "CenteredTrackballMover";
}

ClassicTrackballMover.prototype.Constructor = function(obj, theViewport, theCenteredFlag) {
    MoverBase.prototype.Constructor(obj, theViewport);
    obj.Mover = [];

    const PAN_SPEED           = 1;
    const ZOOM_SPEED          = 1;
    const MAX_DISTANCE_FACTOR = 10.0;

    //////////////////////////////////////////////////////////////////////

    if (theCenteredFlag == undefined) {
        theCenteredFlag = false;
    }

    var _myTrackballBody        = null;
    var _myTrackballOrientation = new Vector3f(0,0,0);
    var _myTrackBallCenter      = new Point3f(0,0,0);

    var _myMousePosX = 0;
    var _myMousePosY = 0;
    var _myCenteredFlag = theCenteredFlag;
    var _myFixedCenter  = false;
    var _myPowerMateButtonState = BUTTON_UP;
    
  
    
    //////////////////////////////////////////////////////////////////////
    //
    // public
    //
    //////////////////////////////////////////////////////////////////////

    obj.name = "ClassicTrackballMover";

    obj.setup = function() {
        setupTrackball(null);
    }

    obj.Mover.onMouseButton = obj.onMouseButton;
    obj.onMouseButton = function(theButton, theState, theX, theY) {
        obj.Mover.onMouseButton(theButton, theState, theX, theY);
        if (theButton == LEFT_BUTTON && theState == BUTTON_DOWN) {
            if (obj.getDoubleLeftButtonFlag()) {
                var myPickedBody = pickBody(theX, theY);
                setupTrackball(myPickedBody);
            }
            _myTrackBallCenter = getTrackballCenter();
        }
    }

    obj.setCentered = function(theFlag) {
        _myCenteredFlag = theFlag;
        setupTrackball(null);
    }

    obj.setTrackedBody = function(theBody) {
        setupTrackball(theBody);
    }

    obj.setTrackballCenter = function(theCenter) {
        _myFixedCenter = true;
        _myTrackBallCenter = theCenter;
        setupTrackball(null);
    }

    obj.rotate = function(theDeltaX, theDeltaY) {
        _myTrackballOrientation.x += theDeltaY * TWO_PI;
        _myTrackballOrientation.y -= theDeltaX * TWO_PI;
        calculateTrackball();
    }

    obj.zoom = function(theDelta) {
        var myZoomFactor =  getDistanceDependentFactor();
        var myWorldTranslation = new Vector3f(0, 0, theDelta * obj.getWorldSize() * myZoomFactor / ZOOM_SPEED);
        obj.update(myWorldTranslation, 0);
    }

    obj.pan = function(theDeltaX, theDeltaY) {
        var myPanFactor =  getDistanceDependentFactor();
        var myWorldSize = obj.getWorldSize();
        var myWorldTranslation = new Vector3f(0, 0, 0);
        myWorldTranslation.x = - theDeltaX * myWorldSize * myPanFactor / PAN_SPEED;
        myWorldTranslation.y = - theDeltaY * myWorldSize * myPanFactor / PAN_SPEED;
        _myTrackballBody = null;
        obj.update(myWorldTranslation, 0);

    }

    obj.onMouseMotion = function(theX, theY) {
        var myDeltaX = (theX-_myMousePosX) / window.width;
        var myDeltaY = - (theY-_myMousePosY) / window.height; // flip Y

        if (obj.getRightButtonFlag()) {
            obj.zoom(-myDeltaY);
        } else if (obj.getMiddleButtonFlag()) {
            obj.pan(myDeltaX, myDeltaY);
        } else if (obj.getLeftButtonFlag()) {
            obj.rotate(myDeltaX, myDeltaY);
        }

        _myMousePosX = theX;
        _myMousePosY = theY;
    }

    
    obj.onAxis = function(theDevice, theAxis, theValue) {
        /*if( _myPowerMateButtonState == BUTTON_UP) {
          _myTrackballOrientation.y += theValue / TWO_PI;
          calculateTrackball();
          } else if( _myPowerMateButtonState == BUTTON_DOWN ) {
          if( theValue != 0 ) {
          obj.zoom(theValue / TWO_PI);
          }
          }*/
    }

    obj.onButton = function(theDevice, theButton, theState) {
        /*switch(theState) {
          case BUTTON_DOWN:
          _myPowerMateButtonState = BUTTON_DOWN;
          break;
          case BUTTON_UP:
          _myPowerMateButtonState = BUTTON_UP;
          break;
          default:
          break;
          }*/
    }

    //////////////////////////////////////////////////////////////////////
    //
    // private
    //
    //////////////////////////////////////////////////////////////////////

    function getDistanceDependentFactor(){
        var d = Math.min(MAX_DISTANCE_FACTOR,
                magnitude(difference(window.camera.globalmatrix.getTranslation(), _myTrackBallCenter)) / obj.getWorldSize());
        return d;
    }

    function setupTrackball(theBody) {
        if (theBody) {
            _myTrackballBody = theBody;
        } else {
            _myTrackballBody = null;
        }

        _myTrackBallCenter = getTrackballCenter();
        var myGlobalPosition = obj.getMoverObject().globalmatrix.getTranslation();
        var myRotation = obj.getMoverObject().globalmatrix.getRotation();
        var myRadiusVector = normalized(difference(myGlobalPosition, _myTrackBallCenter));
        
        if (obj.getMoverObject().globalmatrix.getRow(1).y > 0) {
            _myTrackballOrientation.x = - Math.asin(myRadiusVector.y);
            _myTrackballOrientation.y = Math.atan2(myRadiusVector.x, myRadiusVector.z);
        } else {
            _myTrackballOrientation.x = Math.PI + Math.asin(myRadiusVector.y);
            _myTrackballOrientation.y = Math.atan2(myRadiusVector.x, myRadiusVector.z) - Math.PI;
        }
        
        calculateTrackball();
    }

    function calculateTrackball() {
        var myTrackballRadius = magnitude(difference(obj.getMoverObject().globalmatrix.getTranslation(), _myTrackBallCenter));
        var myX = myTrackballRadius * Math.cos(- _myTrackballOrientation.x) * Math.sin(_myTrackballOrientation.y);
        var myY = myTrackballRadius * Math.sin(- _myTrackballOrientation.x);
        var myZ = myTrackballRadius * Math.cos(- _myTrackballOrientation.x) * Math.cos(_myTrackballOrientation.y);
        var myGlobalMatrix = new Matrix4f(Quaternionf.createFromEuler(_myTrackballOrientation));
        
        myGlobalMatrix.translate(sum(_myTrackBallCenter, new Vector3f(myX, myY, myZ)));
        var myParentMatrix = new Matrix4f(obj.getMoverObject().parentNode.globalmatrix);
        myParentMatrix.invert();
        myGlobalMatrix.postMultiply(myParentMatrix);

        var myDecomposition = myGlobalMatrix.decompose();
        obj.getMoverObject().orientation = myDecomposition.orientation;
        obj.getMoverObject().position = myDecomposition.position;
    }

    // Returns center in global coordinates
    function getTrackballCenter() {
        var myPos = null;
        if (_myCenteredFlag) {
            myPos =  new Point3f(0,0,0);
        }
        else if (_myFixedCenter) {
            myPos =  _myTrackBallCenter;
        }
        else if (_myTrackballBody) {
            myPos =  _myTrackballBody.boundingbox.center;
        } else {
            var myViewVector = product(obj.getMoverObject().globalmatrix.getRow(2).xyz, -1);
            var myPosition   = obj.getMoverObject().globalmatrix.getTranslation();
            var myCenterRay  = new Ray(myPosition, myViewVector);
            var myTrackballRadius = 1;
            var myIntersection = nearestIntersection(obj.getWorld(), myCenterRay);
            if (myIntersection) {
                myTrackballRadius = myIntersection.distance;
            }
            myPos =  sum(myPosition, product(myViewVector, myTrackballRadius));
        }
        
        return myPos;
    }

    function pickBody(theX, theY) {
        // TODO: This is not portrait orientation aware.
        // Implement function:
        // window.screenToWorldSpace(theX, theY, NEAR_PLANE);
        var myPickedBody = null;
        
        var myViewport = obj.getViewport();
        var myPosX = 2 * (theX-myViewport.left) / myViewport.width  - 1;
        var myPosY = - (2 * (theY-myViewport.top) / myViewport.height - 1);
        var myClipNearPos = new Point3f(myPosX, myPosY, -1);
        var myClipFarPos = new Point3f(myPosX, myPosY, +1);

        var myCamera = obj.getViewportCamera();
        var myProjectionMatrix = myCamera.frustum.projectionmatrix;
        myProjectionMatrix.invert();
        myProjectionMatrix.postMultiply(myCamera.globalmatrix);

    	var myWorldNearPos = product(myClipNearPos, myProjectionMatrix);
    	var myWorldFarPos = product(myClipFarPos, myProjectionMatrix);
    	//var myCameraPos = new Point3f(myCamera.globalmatrix.getTranslation());
        var myMouseRay = new Ray(myWorldNearPos, myWorldFarPos);
        var myIntersection = nearestIntersection(obj.getWorld(), myMouseRay);
        if (myIntersection) {
            myPickedBody = myIntersection.info.body;
        } else {
            myPickedBody = window.scene.world;
        }
        //print("  -> You picked trackball object: " + myPickedBody);
        return myPickedBody;
    }
}
