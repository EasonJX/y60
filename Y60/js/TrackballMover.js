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
//   $RCSfile: TrackballMover.js,v $
//   $Author: christian $
//   $Revision: 1.32 $
//   $Date: 2005/04/20 16:49:06 $
//
//=============================================================================

// Possible improvements:
// - Use swept sphere for trackball center detection in stead of stick
// - Adapt fly/zoom/pan-speed to height above ground

use("Trackball.js");
use("MoverBase.js");
use("picking_functions.js");
use("intersection_functions.js");

function TrackballMover(theViewport) {
    this.Constructor(this, theViewport);
}

TrackballMover.prototype.Constructor = function(obj, theViewport) {
    MoverBase.prototype.Constructor(obj, theViewport);
    obj.Mover = [];


    const PAN_SPEED           = 1;
    const ZOOM_SPEED          = 1;
    const MAX_DISTANCE_FACTOR = 10.0;
    const MIN_DISTANCE_FACTOR = 0.1;

    //////////////////////////////////////////////////////////////////////

    var _myTrackball            = new Trackball();
    var _myTrackballCenter      = new Point3f(0,0,0);
    var _prevNormalizedMousePosition = new Vector3f(0,0,0); // [-1..1]

    //////////////////////////////////////////////////////////////////////
    //
    // public
    //
    //////////////////////////////////////////////////////////////////////

    obj.name = "TrackballMover";

    obj.setup = function() {
        var myTrackballBody = obj.getMoverObject().parentNode;
        obj.selectBody(myTrackballBody);
    }

    obj.Mover.onMouseButton = obj.onMouseButton;
    obj.onMouseButton = function(theButton, theState, theX, theY) {
        obj.Mover.onMouseButton(theButton, theState, theX, theY);
        if (theButton == LEFT_BUTTON) {
            if (theState == BUTTON_DOWN) {
                if (obj.getDoubleLeftButtonFlag()) {
                    var myTrackedBody = obj.getMoverObject().parentNode;

                    if (obj.getMoverObject().parentNode.nodeName == "world") {
                        var myPickedBody = pickBody(theX, theY);
                        if (myPickedBody) {
                            myTrackedBody = myPickedBody;
                        }
                    } else {
                        print("  -> Trackball object picking only works with top-level cameras");
                    }
                    obj.selectBody(myTrackedBody);
                    applyRotation();
                }
                _prevNormalizedMousePosition = obj.getNormalizedScreen(theX, theY);
            } else { // Button Up
                _prevNormalizedMousePosition = new Vector3f(0,0,0);
            }
        }
    }

    obj.rotate = function(thePrevMousePos, theCurMousePos) {
        _myTrackball.rotate(thePrevMousePos, theCurMousePos);
        applyRotation();
    }

    obj.rotateByQuaternion = function( theQuaternion ) {
        _myTrackball.setQuaternion( product( theQuaternion, _myTrackball.getQuaternion()));
        applyRotation();
    }

    obj.zoom = function(theDelta) {
        var myWorldSize = obj.getWorldSize();

        // scale the zoom by distance from camera to picked object
        var myZoomFactor =  getDistanceDependentFactor();
        var myScreenTranslation = new Vector3f(0, 0, -theDelta * myWorldSize * myZoomFactor / ZOOM_SPEED);

        obj.update(myScreenTranslation, 0);
    }

    obj.pan = function(theDeltaX, theDeltaY) {
        var myScreenTranslation;
        var myCamera = obj.getViewportCamera();
        if(myCamera.frustum.hfov == 0) {
            // ortho camera
            var myOrthoHeight = (obj.getViewport().height / obj.getViewport().width  ) * myCamera.width;
            myScreenTranslation = new Vector3f(myCamera.width * theDeltaX,
                    myOrthoHeight * theDeltaY,
                    0);
        } else {
            // divide by two to compensate for range [-1,1] => [0,1]
            var myPanFactor =  getDistanceDependentFactor() /2 ;
            var myWorldSize = obj.getWorldSize();
            var myScreenTranslation = new Vector3f(0, 0, 0);
            // negate to move camera (not object)
            myScreenTranslation.x = -theDeltaX * myWorldSize * myPanFactor / PAN_SPEED;
            myScreenTranslation.y = -theDeltaY * myWorldSize * myPanFactor / PAN_SPEED;
        }
        obj.update(myScreenTranslation, 0);
        //Logger.warning("Pan:"+obj.getScreenPanVector());
    }

    obj.onMouseMotion = function(theX, theY) {
        var curNormalizedMousePos = obj.getNormalizedScreen(theX, theY);
        var myDelta = difference(curNormalizedMousePos, _prevNormalizedMousePosition);
        if (obj.getRightButtonFlag()) {
            obj.zoom(myDelta[1]);
        } else if (obj.getMiddleButtonFlag()) {
            obj.pan(myDelta[0], myDelta[1]);
        } else if (obj.getLeftButtonFlag()) {
            obj.rotate(_prevNormalizedMousePosition, curNormalizedMousePos);
        }
        _prevNormalizedMousePosition = curNormalizedMousePos;
    }
    
    obj.selectBody = function(theBody) {
        _myTrackballCenter = getTrackballCenterFromBody(theBody);
        setupTrackball();
    }

    obj.setTrackballCenter = function(theCenter) {
        _myTrackballCenter = theCenter;
        setupTrackball();
    }

    obj.getOrientation = function() {
        return _myTrackball.getQuaternion();
    }

    obj.setOrientation = function(theOrientation) {
        _myTrackball.setQuaternion(theOrientation);
        applyRotation();
    }
    //////////////////////////////////////////////////////////////////////
    //
    // private
    //
    //////////////////////////////////////////////////////////////////////

    function getDistanceDependentFactor(){
        var toObject = difference(obj.getMoverObject().position, _myTrackballCenter);
        var myDistanceFactor =  clamp(magnitude(toObject) / obj.getWorldSize(),
                                MIN_DISTANCE_FACTOR, MAX_DISTANCE_FACTOR);
        return myDistanceFactor;
    }

    function setupTrackball() {
        var myDecomposition = obj.getMoverObject().globalmatrix.decompose();
        _myTrackball.setQuaternion(myDecomposition.orientation);
    }

    function getTrackballCenterFromBody(theBody) {
        if (theBody) {
            if (theBody.boundingbox.isEmpty) {
                return theBody.globalmatrix.getRow(3).xyz;
            } else {
                return theBody.boundingbox.center;
            }
        } else {
            var myViewVector = product(obj.getMoverObject().globalmatrix.getRow(2).xyz, -1);
            var myPosition   = obj.getMoverObject().globalmatrix.getTranslation();
            var myCenterRay  = new Ray(myPosition, myViewVector);

            var myTrackballRadius = 1.0; //_myTrackball.getSphereRadius();
            var myIntersection = nearestIntersection(obj.getWorld(), myCenterRay);
            if (myIntersection) {
            	myTrackballRadius = myIntersection.distance;
            }
            return product(myViewVector, myTrackballRadius);
        }
    }

    obj.getScreenPanVector = function() {
        var myRightVector = obj.getMoverObject().globalmatrix.getRow(0).xyz;
        var myUpVector = obj.getMoverObject().globalmatrix.getRow(1).xyz;
        var myMoverObjectWorldPos = obj.getMoverObject().globalmatrix.getRow(3).xyz;
        
        var myObjectToCenter = difference(myMoverObjectWorldPos, _myTrackballCenter);
        var myPanVector = new Vector3f(dot(myObjectToCenter, myRightVector), dot(myObjectToCenter, myUpVector), 0);
        return myPanVector;
    }

    obj.setTrackballRadius = function( theRadius ) {
        _myTrackball.setSphereRadius( theRadius );
    }

    obj.set = function(theOrientation, theRadius, thePanVector) {
        var myNewMatrix = new Matrix4f();
        myNewMatrix.makeIdentity();
        myNewMatrix.translate(new Vector3f(0,0,theRadius));
        myNewMatrix.postMultiply(new Matrix4f(theOrientation));
        myNewMatrix.translate(_myTrackballCenter);

        var parentMatrixInv = obj.getMoverObject().parentNode.inverseglobalmatrix;
        myNewMatrix.postMultiply(parentMatrixInv);
        // now set the move object's values
        var myDecomposition = myNewMatrix.decompose();

        obj.getMoverObject().position = myDecomposition.position;
        myDecomposition.orientation.normalize();
        obj.getMoverObject().orientation = myDecomposition.orientation;
        _myTrackball.setQuaternion(myDecomposition.orientation);
        obj.getMoverObject().shear = myDecomposition.shear;
        obj.getMoverObject().scale = myDecomposition.scale;
        obj.update(thePanVector, 0);
    }

    function applyRotation() {
        var myPanVector = obj.getScreenPanVector();        
        var myWorldTranslation = obj.rotateWithObject(myPanVector).xyz;
        var myMoverObjectWorldPos = obj.getMoverObject().globalmatrix.getRow(3).xyz;
        var myDistance = distance(difference(myMoverObjectWorldPos, myWorldTranslation), _myTrackballCenter);
        // var deltaRotationMatrix = new Matrix4f(_myTrackball.getQuaternion());

        obj.set(_myTrackball.getQuaternion(), myDistance, myPanVector);
        //Logger.warning("myDistance="+myDistance);
    }

    function pickBody(theX, theY) {
        if (obj.getMoverObject().parentNode.nodeName != "world") {
            print("  -> Trackball object picking only works with top-level cameras");
            return obj.getMoverObject().parentNode;
        }

        // TODO: This is not portrait orientation aware.
        // Implement function:
        // window.screenToWorldSpace(theX, theY, NEAR_PLANE);
        // normalize to [-1..1]
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
        var myMouseRay = new Ray(myWorldNearPos, myWorldFarPos);

        var myWorld = obj.getWorld();
    	var myIntersection = nearestIntersection(myWorld, myMouseRay);
    	if (myIntersection) {
    	    print("  -> You picked trackball object: " + myIntersection.info.body.name);
    	    return myIntersection.info.body;
    	} else {
    	    return obj.getMoverObject().parentNode;
    	}
    }
}
