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
//   $RCSfile: picking_functions.js,v $
//   $Author: martin $
//   $Revision: 1.14 $
//   $Date: 2005/04/12 12:25:24 $
//
//=============================================================================

function Picking(theRenderWindow) {
    this.Constructor(this, theRenderWindow);
}

Picking.prototype.Constructor = function (obj, theRenderWindow) {

    var _myRenderWindow = theRenderWindow;

/*******************************************************/
/**    Public Methods                                 **/
/*******************************************************/
obj.getViewportAt = function(theScreenPixelX, theScreenPixelY, theCanvas) {
    if (!theCanvas) {
        theCanvas = _myRenderWindow.canvas;
    }
    var myViewportCount = theCanvas.childNodes.length;
    for (var i= myViewportCount-1; i >= 0; --i) {
        var myViewport = theCanvas.childNodes[i];
        if (myViewport.nodeName = "viewport" &&
            myViewport.top < theScreenPixelY &&
            myViewport.height + myViewport.top > theScreenPixelY &&
            myViewport.left < theScreenPixelX &&
            myViewport.left + myViewport.width > theScreenPixelX)
        {
            return myViewport;
        }
    }
    return null;
}


/** Find intersection closest to the current camera.
 * An optional world space clipping plane can be specified
 */
obj.pickPosition = function(theScreenPixelX, theScreenPixelY, theClippingPlane) {
    var myClosestPosition = null;
    var myInfo = pickIntersection(theScreenPixelX, theScreenPixelY);
    if (myInfo) {
        var myIntersections = myInfo.info.intersections;
        if (myIntersections.length > 0) {

            var myCameraPos = getCameraPos();
            var myClosestDistance = null; // = distance(myIntersections[0].position,myCameraPos);
            //myClosestPosition = myIntersections[0].position;
            for (var i = 0; i < myIntersections.length; ++i) {
                var myDistance = distance(myIntersections[i].position,myCameraPos);

                if ( theClippingPlane ) {
                    var myDistanceToPlane = signedDistance(myIntersections[i].position, theClippingPlane);
                    if (myDistanceToPlane > 0) {
                        if ( myClosestDistance == undefined || myDistance < myClosestDistance) {
                            myClosestDistance = myDistance;
                            myClosestPosition = myIntersections[i].position;
                        }
                    }
                } else {
                    if ( myClosestDistance == undefined || myDistance < myClosestDistance) {
                        myClosestDistance = myDistance;
                        myClosestPosition = myIntersections[i].position;
                    }
                }
            }
            //print ("closest is "+myInfo.info.body+"@" + myClosestPosition + ", distance = " + myClosestDistance);
        }
    }
    return myClosestPosition;
}


obj.pickBody = function(theScreenPixelX, theScreenPixelY) {
    var myIntersection = pickIntersection(theScreenPixelX, theScreenPixelY);
    if (myIntersection) {
        return myIntersection.info.body;
    } else {
        return null;
    }
}

obj.pickBodyBySweepingSphereFromBodies = function(theScreenPixelX, theScreenPixelY, theSphereRadius, theRootNode)
{
    var myClosestBody = null;
    var myCollisions = obj.pickCollisionsBySweepingSphereFromBodies(theScreenPixelX, theScreenPixelY, theSphereRadius, theRootNode);
    if (myCollisions) {
        if (myCollisions.length > 0) {
            var body = myCollisions[0].body;
            var myCameraPos = getCameraPos();

            var myClosestDistance = distance(body.position,myCameraPos);
            var myClosestBody = body;
            for(var i=1;i<myCollisions.length;++i) {
                body = myCollisions[i].body;
                var myDistance = distance(body.position,myCameraPos);
                if (myDistance < myClosestDistance) {
                    myClosestDistance = myDistance;
                    myClosestBody = body;
                }
            }
        }
    }
    return myClosestBody;
}
/*******************************************************/
/**    Private Methods                                **/
/*******************************************************/
obj.transformClipToWorld = function(theScreenPos) {

    var myProjectionMatrix = new Matrix4f(_myRenderWindow.projectionmatrix);
    myProjectionMatrix.invert();
    myProjectionMatrix.postMultiply(_myRenderWindow.camera.globalmatrix);
    // print (myProjectionMatrix);
    return product(theScreenPos, myProjectionMatrix);
}

obj.transformWorldToClip = function(theWorldPos) {
    var myProjectionMatrix = new Matrix4f(_myRenderWindow.camera.globalmatrix);
    myProjectionMatrix.invert();
    myProjectionMatrix.postMultiply(_myRenderWindow.projectionmatrix);
    return product(theWorldPos, myProjectionMatrix);
}

function getScreenPos(theScreenPixelX, theScreenPixelY) {
    // TODO: This is not portrait orientation aware.
    // Implement function:
    // _myRenderWindow.screenToWorldSpace(theX, theY, NEAR_PLANE);
    var myPosX = 2 * theScreenPixelX / _myRenderWindow.width  - 1;
    var myPosY = - (2 * theScreenPixelY / _myRenderWindow.height - 1);
    var myScreenPos = new Point3f(myPosX, myPosY, -1);
    // print ("myScreenPos:" + myScreenPos);
    return obj.transformClipToWorld(myScreenPos);
}

function getCameraPos() {
    return new Point3f(_myRenderWindow.camera.globalmatrix.getTranslation());
}

function getCameraToScreenRay(theScreenPixelX, theScreenPixelY) {
    var myScreenPos = getScreenPos(theScreenPixelX, theScreenPixelY);
    var myCameraPos = getCameraPos();
    return new Ray(myCameraPos, myScreenPos);
}

function getRayThroughScreen(theScreenPixelX, theScreenPixelY) {
    var myScreenPos = getScreenPos(theScreenPixelX, theScreenPixelY);
    var myPosX = 2 * theScreenPixelX / _myRenderWindow.width  - 1;
    var myPosY = - (2 * theScreenPixelY / _myRenderWindow.height - 1);
    var myFarEyePos = new Point3f(myPosX, myPosY, 1);
    var myFarPos = obj.transformClipToWorld(myFarEyePos);

    var myViewRay = new Ray(myScreenPos, myFarPos);
    // print (myViewRay);
    return myViewRay;
}

obj.getLineSegmentThroughScreen = function(theScreenPixelX, theScreenPixelY) {
    var myScreenPos = getScreenPos(theScreenPixelX, theScreenPixelY);
//    var myCameraPos = getCameraPos();
//    return new Ray(myCameraPos, myScreenPos);
    var myPosX = 2 * theScreenPixelX / _myRenderWindow.width  - 1;
    var myPosY = - (2 * theScreenPixelY / _myRenderWindow.height - 1);

    var myEyePos = new Point3f(myPosX, myPosY, 1);
    var myFarPos = obj.transformClipToWorld(myEyePos);

    myEyePos = new Point3f(myPosX, myPosY, -1);
    var myNearPos = obj.transformClipToWorld(myEyePos);

    var myViewLineSegment = new LineSegment(myNearPos, myFarPos);
    //print (myViewLineSegment);
    return myViewLineSegment;
}

function pickIntersection(theScreenPixelX, theScreenPixelY) {
//    var myCameraToScreenRay = getCameraToScreenRay(theScreenPixelX, theScreenPixelY);
    var myLineSegment = obj.getLineSegmentThroughScreen(theScreenPixelX, theScreenPixelY);
    var myWorld = getDescendantByTagName(_myRenderWindow.scene.dom, "world", true);
    return nearestIntersection(myWorld, myLineSegment);
}

obj.pickCollisionsBySweepingSphereFromBodies = function(theScreenPixelX, theScreenPixelY, theSphereRadius, theRootNode)
{

    var myScreenPos = getScreenPos(theScreenPixelX, theScreenPixelY);
    var myPosX = 2 * theScreenPixelX / _myRenderWindow.width  - 1;
    var myPosY = - (2 * theScreenPixelY / _myRenderWindow.height - 1);
    var myFarPlanePos = new Point3f(myPosX, myPosY, 1);
    myFarPlanePos = obj.transformClipToWorld(myFarPlanePos);

    //print("ScreenP " + myScreenPos + " FarPlaneP " + myFarPlanePos);
    var mySphere = new Sphere(myScreenPos, theSphereRadius);

    var myMotion = new Vector3f(difference(myFarPlanePos,myScreenPos));

    return Scene.collideWithBodies(theRootNode, mySphere, myMotion);
}

}
