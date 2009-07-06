/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2008, ART+COM AG Berlin, Germany <www.artcom.de>
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
// Description: TODO  
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
// Y60 JSSL - Y60 JavaScript Standard Library
// Content:
//     - JavaScript helper functions
//==============================================================================

var PI_2 = Math.PI / 2;
var PI_4 = Math.PI / 4;
var PI_180 = Math.PI / 180;
var TWO_PI = Math.PI * 2.0;

// This only works with native js objects or
// properly wrapped hosted objects.
// 
// However, neither the prototype chain of objects
// nor the scope chain of function objects will be copied. Thus,
// "clone" is a rather euphemistic name for this function.
// 
// Looking at the details, things become a bit ugly, though:
//
// JSNode objects for example react differently when properties
// are accessed through the "." operator, compared to "[]". The Code:
//
// someNode["nodeValue"]
//
// will throw a DOMException, while with the dot notation: 
// 
// var i = "nodeValue";
// var ival = node.i
//
// anything works fine. Thus, we provide two variants of clone().
// (TA 2007-10-14)

function clone(theObject, theMode) {
    var myNewObject = [];

    if (theObject instanceof Node) {
	var attrs = theObject.attributes;
        for (var i = 0; i <attrs.length; i++) {
	    myNewObject[attrs[i].nodeName] = attrs[i].nodeValue;	
        }
	return myNewObject;
    }

    if (!theMode) {
        for (i in theObject) {
            if (typeof theObject[i] == "object") {
                myNewObject[i] = clone(theObject[i], theMode);
            } else {
                myNewObject[i] = theObject[i];
            }
        }
    } else {
        for (i in theObject) {
            if (typeof theObject.i == "object") {
                myNewObject.i = clone(theObject.i, theMode);
            } else {
                myNewObject.i = theObject.i;
            }
        }
    }
    return myNewObject;
}

// WTF?
function getFocalLength(theHfov) {
    return (35 / (2 * Math.tan(theHfov * Math.PI / 360)));
}

function getHPR(theMatrix) {
    // From "3D Engine Design", page 19
    var myH = Math.asin(theMatrix[2][0]);
    var myP = 0;
    var myR = 0;
    if (myH < PI_2) {
        if (myH > - PI_2) {
            myP = Math.atan2(-theMatrix[2][1], theMatrix[2][2]);
            myR = Math.atan2(-theMatrix[1][0], theMatrix[0][0]);
        } else {
            // not a unique solution
            myP = -Math.atan2(theMatrix[0][1], theMatrix[1][1]);
            myR = 0;
        }
    } else {
        // not a unique solution
        myP = Math.atan2(theMatrix[0][1], theMatrix[1][1]);
        myR = 0;
    }

    return new Vector3f(myH, myP, myR);
}

// get normal from three points
function calcNormal(thePoint1, thePoint2, thePoint3) {
    var myVector1 = difference(thePoint1, thePoint2);
    var myVector2 = difference(thePoint2, thePoint3);

    return normalized(cross(myVector1, myVector2));
}

function fmod(a, b) {
    return (a - (Math.floor(a / b) * b));
}

function degFromRad(theRadiant) {
    return theRadiant * (180.0 / Math.PI);
}

function radFromDeg(theDegree) {
    return theDegree * (Math.PI / 180.0);
}

function angleBetween(a,b, theHalfCircle) {
    var d = a - b;
    while (d > theHalfCircle) {
        d -= theHalfCircle * 2;
    }
    while (d < -theHalfCircle) {
        d += theHalfCircle*2;
    }
    return d;
}

function radBetween(a,b) {
    return angleBetween(a,b, Math.PI);
}
function degBetween(a,b) {
    return angleBetween(a,b, 180);
}



function clamp(theValue, theMin, theMax) {
    if (theValue < theMin) {
        return theMin;
    }
    if (theValue > theMax) {
        return theMax;
    }
    return theValue;
}

function tanh(x) {
    var myExp = Math.exp(x);
    var myNegExp = Math.exp(-x)
    return (myExp-myNegExp) / (myExp+myNegExp);
}

function stringToArray(s) {
    return s.substring(1, s.length - 1).split(",");
}

function stringToByteArray(theString) {
    var myResult = [];
    for (var i = 0; i < theString.length; ++i) {
        myResult.push(theString.charCodeAt(i));
    }
    return myResult;
}

// Expects theDateString to be in the format "2000-1-31"
function parseDate(theDateString) {
    var myDate = theDateString.split("-");
    return new Date(myDate[0], myDate[1] - 1, myDate[2]);
}

var COLOR_HEX_STRING_PATTERN = /^[0-9A-Fa-f]{6}/;

// use like this: asColor("00BFA3", 1);
function asColor(theHexString, theAlpha) {

    if (!theHexString) {
        throw new Exception("asColor: theHexString is not defined!");
    }

    if (theHexString instanceof Vector4f) {
	Logger.trace("asColor: is Vector4f:" + theHexString);
	return theHexString;
    }

    Logger.trace(typeof theHexString);

    if (theAlpha == undefined) {
        theAlpha = 1;
    }

    if (theHexString instanceof Vector3f) {
	Logger.trace("asColor: is Vector3f:" + theHexString);
        return new Vector4f(theHexString[0], theHexString[1], theHexString[2], theAlpha);
    }

    if (COLOR_HEX_STRING_PATTERN.test(theHexString)) {
	Logger.trace("asColor: evaluating " + theHexString);
        var myRed   = eval("0x" + theHexString[0] + theHexString[1]);
        var myGreen = eval("0x" + theHexString[2] + theHexString[3]);
        var myBlue  = eval("0x" + theHexString[4] + theHexString[5]);
        return new Vector4f(myRed / 255, myGreen / 255, myBlue / 255, theAlpha);
    } else {
	var retval = eval(theHexString);
	Logger.trace("asColor: eval " + theHexString + " ergibt " + retval);
	return new Vector4f(retval);
    }
}

// returns a string represenation of an (nested) array
function arrayToString(a) {
    var myString = "[";
    for (var i = 0; i < a.length; ++i) {
        var myElement = a[i];
        if (a[i] && a[i].constructor && a[i].constructor == Array) {
            myString += arrayToString(a[i]);
        } else {
            myString += a[i];
        }

        if (i < a.length - 1) {
            myString += ",";
        }
    }
    myString += "]";
    return myString;
}

// returns a precision formated string from a Vector3f list
function formatVector3f(theVector, thePrecision) {

    var myString = "";
    for (var i = 0; i < theVector.length; ++i) {

        if (i != 0) {
            myString += ",";
        }

        myString += "[" + theVector[i].x.toFixed(thePrecision) + ",";
        myString += theVector[i].y.toFixed(thePrecision) + ",";
        myString += theVector[i].z.toFixed(thePrecision) + "]";
    }

    return myString;
}

// returns a precision formated string from a Vector2f list
function formatVector2f(theVector, thePrecision) {

    var myString = "";
    for (var i = 0; i < theVector.length; ++i) {

        if (i != 0) {
            myString += ",";
        }

        myString += "[" + theVector[i].x.toFixed(thePrecision) + ",";
        myString += theVector[i].y.toFixed(thePrecision) + "]";
    }

    return myString;
}

// NOTE: normalized vectors required
function getOrientationFromDirection(theViewVector, theUpVector) {
    var myMatrix = new Matrix4f();
    var myUpVector = normalized(projection(theUpVector, new Planef(theViewVector, [0,0,0])));

    var myRightVector = cross(theViewVector, myUpVector);
    myMatrix.setRow(0, new Vector4f(myRightVector[0], myRightVector[1], myRightVector[2], 0));
    myMatrix.setRow(1, new Vector4f(myUpVector[0], myUpVector[1], myUpVector[2], 0));
    myMatrix.setRow(2, new Vector4f(-theViewVector[0], -theViewVector[1], -theViewVector[2], 0));

    return myMatrix.decompose().orientation;
}

// Returns a string containing all the attributes of element E
function listAttributes(E) {
    var myResult = ' ';
    for (var i = 0; i < E.attributes.length; ++i) {
        myResult+=E.attributes[i].nodeName+'="'+E.attributes[i].nodeValue+'" ';
    }
    return myResult;
}

// Dumps element E to console
function dumpElementShallow(E) {
    dumpElement(E,'',0);
}

// Dumps element E and maxDepth levels of children to console
function dumpElementLevels(E,maxDepth) {
    dumpElement(E,'',maxDepth);
}

// Dumps element E with theMaxDepth levels of children using theSpace as base indent
function dumpElement(E,theSpace,theMaxDepth) {
    if (E) {
        if (E.nodeType==Node.ELEMENT_NODE) {
            var myId = E.getAttribute('id');
            print(theSpace+'<'+E.nodeName+listAttributes(E)+'>');
            if (theMaxDepth!=0) {
                for (var i=0; i < E.childNodes.length; ++i) {
                    dumpElement(E.childNodes[i], theSpace+'    ',theMaxDepth-1);
                    //dumpElement(E.childNodes.item(i), theSpace+'    ',theMaxDepth-1);
                }
            }
            print(theSpace+'<'+E.nodeName+'/>');
        } else if (E.nodeType==E.TEXT_NODE) {
            print(E.nodeValue);
        }
    }
}

// Adjust Node.id (and it's descendants) to be unique
function adjustNodeId(theNode, theDeepAdjustFlag) {
    if ("id" in theNode) {
        theNode.id = createUniqueId();
    }
    if (theDeepAdjustFlag == undefined) {
        theDeepAdjustFlag = false;
    }
    if (theDeepAdjustFlag) {
        for (var i = 0; i < theNode.childNodes.length; ++i) {
            adjustNodeId(theNode.childNodes[i], theDeepAdjustFlag);
        }
    }
}

function adjustNodeIds(theNode) {
    adjustNodeId(theNode, true);
}

//searches for a descendant of theNode (must be in DOM below theNode)
function getDescendantById(theNode, theId, doDeepSearch) {
    if (doDeepSearch == undefined || !doDeepSearch) {
        doDeepSearch = false;
    }
    Logger.warning("getDescendantById is deprecated");
    dumpstack();
    var myResult = theNode.getNodesByAttribute("", "id",  theId, doDeepSearch);    
    return myResult.length == 0 ? null:myResult[0];
}

function getDescendantByName(theNode, theName, doDeepSearch) {
    if (doDeepSearch == undefined || !doDeepSearch) {
        doDeepSearch = false;
    }
    Logger.warning("getDescendantByName is deprecated");
    dumpstack();
    var myResult = theNode.getNodesByAttribute("", "name",  theName, doDeepSearch);    
    return myResult.length == 0 ? null:myResult[0];
}

function getDescendantsByName(theNode, theName, doDeepSearch) {
    if (doDeepSearch == undefined || !doDeepSearch) {
        doDeepSearch = false;
    }
    Logger.warning("getDescendantsByName is deprecated");
    dumpstack();
    var myResult = theNode.getNodesByAttribute("", "name",  theName, doDeepSearch);
    return myResult;
}

// Recursivly searches theNode for the first element that has theAttribute.
// Can search deep or shallow depending on the value of doDeepSearch
function getDescendantByAttributeName(theNode, theAttribute, doDeepSearch) {
    if (doDeepSearch == undefined || !doDeepSearch) {
        doDeepSearch = false;
    }
    Logger.warning("getDescendantByAttributeName is deprecated");
    dumpstack();
    var myResult = theNode.getNodesByAttribute("", theAttribute, "", doDeepSearch);    
    return myResult.length == 0 ? null:myResult[0];
}

// Recursivly searches theNode for the first element that has theAttribute with value theValue.
// Can search deep or shallow depending on the value of doDeepSearch
function getDescendantByAttribute(theNode, theAttribute, theValue, doDeepSearch) {
    if (doDeepSearch == undefined || !doDeepSearch) {
        doDeepSearch = false;
    }
    Logger.warning("getDescendantByAttribute is deprecated");
    dumpstack();
    var myResult = theNode.getNodesByAttribute("", theAttribute, theValue, doDeepSearch);    
    return myResult.length == 0 ? null:myResult[0];
}

// Recursivly searches theNode for all elements that have theAttribute with value theValue.
// Can search deep or shallow depending on the value of doDeepSearch
function getDescendantsByAttribute(theNode, theAttribute, theValue, doDeepSearch) {
    if (doDeepSearch == undefined || !doDeepSearch) {
        doDeepSearch = false;
    }
    Logger.warning("getDescendantsByAttribute is deprecated");
    dumpstack();
    var myResult = theNode.getNodesByAttribute("", theAttribute, theValue, doDeepSearch);    
    return myResult;
}

function getDescendantsByAttributeName(theNode, theAttribute, doDeepSearch) {
    if (doDeepSearch == undefined || !doDeepSearch) {
        doDeepSearch = false;
    }
    Logger.warning("getDescendantsByAttributeName is deprecated");
    dumpstack();
    var myResult = theNode.getNodesByAttribute("", theAttribute, "", doDeepSearch);    
    return myResult;
}

// Recursivly search for the first element by tagname
function getDescendantByTagName(theNode, theTagName, doDeepSearch) {
    if (doDeepSearch == undefined || !doDeepSearch) {
        doDeepSearch = false;
    }
    Logger.warning("getDescendantByTagName is deprecated");
    dumpstack();
    var myResult = theNode.getNodesByTagName(theTagName, doDeepSearch);    
    return myResult.length == 0 ? null:myResult[0];
}

// Recursivly search for all elements by tagname
function getDescendantsByTagName(theNode, theTagName, doDeepSearch) {
    if (doDeepSearch == undefined || !doDeepSearch) {
        doDeepSearch = false;
    }
    Logger.warning("getDescendantsByTagName is deprecated");
    dumpstack();
    var myResult = theNode.getNodesByTagName(theTagName, doDeepSearch);        
    return myResult;
}

function getChildElementNodes(theNode, theFilterOperation, theFilterNodeName) {
    Logger.warning("getChildElementNodes is deprecated");
    dumpstack();
    var myChildElementNodes = [];
    var i;

    try {
        if (theNode) {
            for (var n = 0; n < theNode.childNodes.length; n++) {
                if (theNode.childNodes[n].nodeType==Node.ELEMENT_NODE) {
                    switch (theFilterOperation) {
                    case 'ignore':
                        if (theFilterNodeName != theNode.childNodes[n].nodeName) {
                            myChildElementNodes.push(theNode.childNodes[n]);
                        }
                        break;
                    case 'match':
                        if (theFilterNodeName == theNode.childNodes[n].nodeName) {
                            myChildElementNodes.push(theNode.childNodes[n])
                                }
                        break;
                    case 'ignore-list':
                        for (i = 0; i < theFilterNodeName.length; i++) {
                            if (theFilterNodeName[i] != theNode.childNodes[n].nodeName) {
                                myChildElementNodes.push(theNode.childNodes[n]);
                            }
                        }
                        break;
                    case 'match-list':
                        for (i = 0; i < theFilterNodeName.length; i++) {
                            if (theFilterNodeName[i] == theNode.childNodes[n].nodeName) {
                                myChildElementNodes.push(theNode.childNodes[n]);
                            }
                        }
                        break;
                    case '':
                    default:
                        myChildElementNodes.push(theNode.childNodes[n]);
                    }
                }
            }
        }
    } catch (ex) {
        print('Exception in getChildElementNodes('+theNode+', '+theFilterOperation+', '+theFilterNodeName+'):\n'+ex);
    }

    return myChildElementNodes;
}

function findNodeByNameChecked(theRootNode, theName) {
    var myResult = theRootNode.find(".//*[@name='"+theName+"']");
    if (!myResult) {
        Logger.error("findNodeByNameChecked failed, see stack trace:");
        dumpstack();
        Logger.fatal("Could not find a node with name '"+theName+"'");
    }
    return myResult;
}


function removeAttributeByName(theNode, theAttributeName) {
    var myCopyNode   = Node.createElement(theNode.nodeName);
    var myAttributes = theNode.attributes;
    for (var j = 0; j < myAttributes.length; ++j) {
        var myName = myAttributes[j].nodeName;
        if (myName != theAttributeName) {
            myCopyNode[myName] = theNode[myName];
        }
    }
    var myChildNodesLength = theNode.childNodesLength();
    for (var j = myChildNodesLength - 1; j >= 0; --j) {
        myCopyNode.appendChild(theNode.childNode(j));
    }
    var myParentNode = theNode.parentNode;
    if (myParentNode) {
        myParentNode.appendChild(myCopyNode);
        myParentNode.removeChild(theNode);
    }
    return myCopyNode;
}

function removeAllAttributes(theNode) {
    var myCopyNode   = Node.createElement(theNode.nodeName);
    var myChildNodesLength = theNode.childNodesLength();
    for (var j = myChildNodesLength - 1; j >= 0; --j) {
        myCopyNode.appendChild(theNode.childNode(j));
    }
    var myParentNode = theNode.parentNode;
    if (myParentNode) {
        myParentNode.appendChild(myCopyNode);
        myParentNode.removeChild(theNode);
    }
    return myCopyNode;
}

function getPropertyValue() {
    var myNode = thePropertiesNode.find(".//*[@name = '" + theProperty + "']");
    if (!myNode) {
        return null;
    }
    if (myNode.nodeName != theDataType) {
        Logger.error("Property "+theProperty+" is not a "+theDataType);
        return null;
    }
    return myNode.childNode('#text').nodeValue;
}

function setPropertyValue(thePropertiesNode, theDataType, theProperty, theValue) {
    var myNode = thePropertiesNode.find(".//*[@name = '" + theProperty + "']");
    if (!myNode) {
        myNode = Node.createElement(theDataType);
        thePropertiesNode.appendChild(myNode);
        myNode.name = theProperty;
        myNode.appendChild(Node.createTextNode(theValue));
    } else {
        if (myNode.nodeName != theDataType) {
            Logger.error("Property "+theProperty+" is not a "+theDataType);
            return null;
        }
        myNode.childNode('#text').nodeValue = theValue;
    }
}

function getTimestamp() {
    var myDate  = new Date();
    var myYear  = myDate.getFullYear();
    var myMonth = myDate.getMonth()+1;
    var myDay   = myDate.getDate();
    var myHour  = myDate.getHours();
    var myMin   = myDate.getMinutes();
    var mySec   = myDate.getSeconds();

    return myYear + "_" +
        ((myMonth < 10) ? "0" : "") + myMonth + "_" +
        ((myDay   < 10) ? "0" : "") + myDay   + "-" +
        ((myHour  < 10) ? "0" : "") + myHour  + "_" +
        ((myMin   < 10) ? "0" : "") + myMin   + "_" +
        ((mySec   < 10) ? "0" : "") + mySec;
}

function padStringFront(theString, thePaddingChar, theLength) {
    if (typeof(theString) != "string") {
        //print("converting from " + typeof(theString));
        theString = String(theString);
    }
    while (theString.length < theLength) {
        theString = thePaddingChar + theString;
    }
    return theString;
}

function padStringBack(theString, thePaddingChar, theLength) {
    if (typeof(theString) != "string") {
        //print("converting from " + typeof(theString));
        theString = String(theString);
    }
    while (theString.length < theLength) {
        theString = theString + thePaddingChar;
    }
    return theString;
}

function hexToNum(theHex) {
    return parseInt(theHex, 16);
}

function numToHex(theNum, theDigitCount) {
    var myHex = theNum.toString(16);
    if (theDigitCount != undefined) {
        while (myHex.length < theDigitCount) {
            myHex = "0" + myHex;
        }
    }
    return myHex;
}

function countNodes(theNode) {
    var myCount = 1;
    for (var i = 0; i < theNode.childNodes.length; ++i) {
        myCount += countNodes(theNode.childNode(i));
    }
    return myCount;
}

function asMemoryString(theKBytes) {
    if (theKBytes < 1024) {
        return theKBytes.toFixed(2) + "K";
    } else if (theKBytes < 1048576) {
        return (theKBytes / 1024).toFixed(2) + "M";
    } else if (theKBytes < 1073741824) {
        return (theKBytes / 1048576).toFixed(2) + "G";
    }
}

function byteAsMemoryString(theBytes) {
    if (theBytes < 1024) {
        return theBytes;
    } else if (theBytes < 1048576) {
        return (theBytes / 1024).toFixed(2) + "K";
    } else if (theBytes < 1073741824) {
        return (theBytes / 1048576).toFixed(2) + "M";
    } else if (theBytes < 1073741824 * 1024) {
        return (theBytes / (1048576 * 1024)).toFixed(2) + "G";
    }
}


// Removes space, newline and tab characters from front and back of a string
function trim(theString) {
    var outString;
    var frontIndex = 0;
    var backIndex = theString.length - 1;
    while (theString.charAt(frontIndex) == " " || theString.charAt(frontIndex) == "\t" ||
           theString.charAt(frontIndex) == "\n" || theString.charAt(frontIndex) == "\r")
    {
        frontIndex++;
    }
    while (theString.charAt(backIndex) == " " || theString.charAt(backIndex) == "\t" ||
           theString.charAt(backIndex) == "\n" || theString.charAt(backIndex) == "\r" )
    {
        backIndex--;
    }
    return frontIndex > backIndex ? "" : theString.substring(frontIndex, (backIndex + 1));
}
function nextPowerOfTwo(n) {
    var myPowerOfTwo = 1;
    while (myPowerOfTwo < n) {
        myPowerOfTwo <<= 1;
    }
    return myPowerOfTwo;
}


function parseXML(theFilename) {
    if (!fileExists(theFilename)) {
        print("### ERROR: parseXML(): File " + theFilename + " does not exist.");
        return null;
    }
    var myDocument = new Node();
    myDocument.parseFile(theFilename);
    return myDocument.firstChild;
}

function formatTimecode(theTimestamp) {
    theTimestamp = Number(theTimestamp);
    var myHours   = 0;
    var myMinutes = 0;
    var mySeconds = 0;

    if (theTimestamp >= 3600) {
        myHours = (theTimestamp - theTimestamp % 3600) / 3600;
        theTimestamp = theTimestamp % 3600;
    }

    if (theTimestamp >= 60) {
        myMinutes = (theTimestamp - theTimestamp % 60) / 60;
        theTimestamp = theTimestamp % 60;
    }

    mySeconds = theTimestamp.toFixed(0);

    var myTC  = (myHours < 10 ? "0" + myHours : myHours) + ":";
    myTC += (myMinutes < 10 ? "0" + myMinutes : myMinutes) + ":";
    myTC += (mySeconds < 10 ? "0" + mySeconds : mySeconds);

    return myTC;
}

function randomBetween(theMin, theMax) {
    return Math.random() * (theMax - theMin) + theMin;
}

function randomInteger(theMin, theMax) {
    return Math.round(Math.random() * (theMax - theMin) + theMin) ;
}
function randomElement(theArray) {
    return theArray[Math.floor(randomBetween(0, theArray.length-0.000001))];
}

var _ourLastRandomNumber_ = 0;
function deterministicRandom() {
    _ourLastRandomNumber_ = (1277 * _ourLastRandomNumber_ + 1) % 131072;
    return _ourLastRandomNumber_ / 131071;
}

function removeElement(theArray, theIndex) {
    return theArray.slice(0, theIndex).concat(theArray.slice(theIndex + 1, theArray.length));
}

function getFrustumFrame(theFrustum, theDepth) {
    //TODO myFrame should be a Box2f
    var myFrame = {};
    //print (theFrustum + "," + theDepth);
    myFrame.left = theDepth * theFrustum.left / theFrustum.near;
    myFrame.right = theDepth * theFrustum.right / theFrustum.near;
    myFrame.top = theDepth * theFrustum.top / theFrustum.near;
    myFrame.bottom = theDepth * theFrustum.bottom / theFrustum.near;
    return myFrame;
}

function getWorldPosition(theBody) {
    return theBody.globalmatrix.getTranslation();
}

function setWorldPosition(theBody, theWorldPosition) {
    var myT = new Matrix4f(theBody.parentNode.globalmatrix);
    myT.invert();
    theBody.position = product(theWorldPosition, myT);
}

function transformClipToWorld(theClipPos, theCamera) {
    // [sh] - beware of great float values
    var myProjectionMatrix = new Matrix4f(theCamera.frustum.projectionmatrix);
    myProjectionMatrix.invert();
    myProjectionMatrix.postMultiply(theCamera.globalmatrix);
    return product(theClipPos, myProjectionMatrix);
}


// transform screen coordinates to world coordinates (i.e. world coordinate of
// the given clipping plane)
// theClipZ: -1 for Near Plane, 1 for Far Plane
function transformScreenToWorld(theScreenPixelX, theScreenPixelY, theViewport, theClipZ) {
    Logger.trace("transformScreenToWorld(" + theScreenPixelX
                 + ", " + theScreenPixelY + "," + theClipZ);
    if (theClipZ == undefined) {
        Logger.trace("ClipZ not defined, using default value");
        theClipZ = -1;
    }
    var myViewportPixelX = theScreenPixelX - theViewport.left;
    var myViewportPixelY = theScreenPixelY - theViewport.top;
    Logger.trace("Pixelcoordinates in viewport: [" + 
                 myViewportPixelX + ", " + 
                 myViewportPixelY + "]");
    var myClipPosX = 2 * myViewportPixelX/theViewport.width  - 1;
    var myClipPosY = - (2 * myViewportPixelY/theViewport.height - 1);
    var myClipCoordinates = new Point3f(myClipPosX, myClipPosY, theClipZ);
    Logger.trace("ClipCoordinates: " + myClipCoordinates);
    return transformClipToWorld(myClipCoordinates, window.scene.world.getElementById( theViewport.camera ));
}

function transformScreenAlignedToWorld(theScreenPixelX, theScreenPixelY, theZ, theViewport) {
    var myScreenPos = transformScreenToWorld(theScreenPixelX, theScreenPixelY, theViewport);
    var myCamera = theViewport.getElementById(theViewport.camera);
    var myRay = new Ray(myCamera.position, myScreenPos);
    return intersection(myRay, new Planef(new Vector3f(0,0,-1), theZ));
}

function easeInOut(theValue) {
    return theValue*theValue*(3-2*theValue);
}

function easeOut(theValue) {
    return 2*easeInOut((theValue+1)/2)-1;
}

function easeIn(theValue) {
    return 2*easeInOut(theValue/2);
}
function gaussianBox_Mueller(theMean, theDeviation) {
	var x1, x2, w, y1;
	do {
		x1 = 2.0 * Math.random() - 1.0;
		x2 = 2.0 * Math.random() - 1.0;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.0 );

	w = Math.sqrt( (-2.0 * Math.log( w ) ) / w );
	y1 = x1 * w;

	return( theMean + y1 * theDeviation );
}
function gaussianRandom() {
    var myGaussianRandom = gaussianBox_Mueller(0.5, 0.5)
    while (myGaussianRandom >1.0 || myGaussianRandom < 0.0) {
        myGaussianRandom = gaussianBox_Mueller(0.5, 0.5)
    }
    return myGaussianRandom;
}

function convertQuatToEuler(q) {
    var q1 = product(q, new Quaternionf(1,0,0,0));
    var sqw = q1[3]*q1[3];
    var sqx = q1[0]*q1[0];
    var sqy = q1[1]*q1[1];
    var sqz = q1[2]*q1[2];
    var unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
    var test = q1[0]*q1[1] + q1[2]*q1[3];
    if (test > 0.499*unit) { // singularity at north pole
        heading = 2 * Math.atan2(q1[0],q1[3]);
        attitude = Math.PI/2;
        bank = 0;
        return new Vector3f(heading, bank, attitude);
    }
    if (test < -0.499*unit) { // singularity at south pole
        heading = -2 * Math.atan2(q1[0],q1[3]);
        attitude = -Math.PI/2;
        bank = 0;
    }
    var heading = Math.atan2(2*q1[1]*q1[3]-2*q1[0]*q1[2] , sqx - sqy - sqz + sqw);
    var attitude = Math.asin(2*test/unit);
    var bank = -Math.atan2(2*q1[0]*q1[3]-2*q1[1]*q1[2] , -sqx + sqy - sqz + sqw)
    return new Vector3f(heading, bank, attitude);
}

// The practical motivation for currying is that very often the functions
// you get by supplying some but not all of the arguments to a curried function are useful.
// Intuitively speaking: 'If you fix some arguments,
// you get a function of the remaining arguments.'
// P.S.: It's 'Schoenfinkeln' in german
function curry() {
    var theFunction = arguments[0];
    var theCurry = Array.prototype.slice.call(arguments, 1);

    return function() {
        var myDirectArguments = Array.prototype.slice.call(arguments);
        var myArguments = theCurry.concat(myDirectArguments);
        return theFunction.apply(this, myArguments);
    };
}

function rcurry() {
    var theFunction = arguments[0];
    var theCurry = Array.prototype.slice.call(arguments, 1);

    return function() {
        var myDirectArguments = Array.prototype.slice.call(arguments);
        var myArguments = myDirectArguments.concat(theCurry);
        return theFunction.apply(this, myArguments);
    };
}


// Function that will reflect a camera on a given plane and return
// the new camera position and orientation.
// Useful for mirroring and reflection
function createMirrorCamera(theCamera, thePlane) {
    var myPlaneNormal = thePlane.normal;
    var myInvertedPlaneNormal = product(myPlaneNormal, -1);
    
    var myMatrix = theCamera.globalmatrix;
    var myRight = myMatrix.getRow(0);
    var myUp = myMatrix.getRow(1);
    var myFwd = myMatrix.getRow(2);
    var myPos = myMatrix.getRow(3);

    // Code to reflect a camera position on a plane

    var myCamPosVec = new Vector3f(myPos.x, myPos.y, myPos.z)
    var myCamPosPoi = new Point3f(myPos.x, myPos.y, myPos.z)
    var myCamUpVec  = new Vector3f(myUp.x, myUp.y, myUp.z);
    var myCamRightVec = new Vector3f(myRight.x, myRight.y, myRight.z);
    

    // reflect cameraposition
    var myPlaneProjectedCamPosLine = new Line(myCamPosPoi, myInvertedPlaneNormal); 
    var myCameraPlaneProjectionPos = intersection(myPlaneProjectedCamPosLine, thePlane);
    var myCameraPlaneProjectionVec = difference(myCameraPlaneProjectionPos, myCamPosVec);
    myCameraPlaneProjectionVec = product(myCameraPlaneProjectionVec,2);
    var myNewCameraPos = sum(myCamPosVec, myCameraPlaneProjectionVec);

    // reflect camera up
    var myCameraUpVec = sum(myCamPosVec, normalized(myCamUpVec));
    var myCameraUpPoi = new Point3f(myCameraUpVec.x, myCameraUpVec.y,myCameraUpVec.z);
	
    var myPlaneProjectedCamUpLine = new Line(myCameraUpPoi, myInvertedPlaneNormal); 
    var myCameraPlaneProjectionUpPos = intersection(myPlaneProjectedCamUpLine, thePlane);
    
    var myCameraPlaneProjectionUpVec = difference(myCameraPlaneProjectionUpPos, myCameraUpVec);
    myCameraPlaneProjectionUpVec = product(myCameraPlaneProjectionUpVec,2);
    var myNewCameraUpVec = sum(myCameraUpVec, myCameraPlaneProjectionUpVec);
    
    var myNewUpVector = normalized(difference(myNewCameraUpVec, myNewCameraPos));
    
    
    // reflect the view vector across the plane
    var myViewVector = normalized( new Vector3f( -myFwd.x, -myFwd.y, -myFwd.z ) );
    var myProjection = projection( myViewVector, thePlane.normal );
    var myNewCameraView = normalized( sum( myViewVector, product( myProjection, -2 ) ) );
    
    var myOrientation = getOrientationFromDirection(myNewCameraView, myNewUpVector);

    return {orientation : myOrientation, position : myNewCameraPos};
}

function lerp(t, v, w) {
    return v + t *(w-v);
}


/*
 * RankedFeature conversion
 */
function stringRankedFeature(theRankedFeature) {

    if (theRankedFeature == null) {
        return "";
    }

    var myString = theRankedFeature.rank + "[";
    for (var i = 0; i < theRankedFeature.features.length; ++i) {
        if (i > 0) {
            myString += ",";
        }
        myString += theRankedFeature.features[i];
    }
    myString += "]";

    return myString;
}

function parseRankedFeature(theString) {

    var myRegex = new RegExp("^\([0-9]*\)\\[\(.*\)\\]$");
    var myMatch = myRegex.exec(theString);
    if (!myMatch || myMatch.length != 3) {
        return null;
    }

    var myRank = new Number(myMatch[1]);
    var myFeatures = myMatch[2].split(",");
    var myRankedFeature = {rank:myRank, features:myFeatures};
    //print(theString, stringRankedFeature(myRankedFeature));

    return myRankedFeature;
}


/*
 * VectorOfRankedFeature conversion
 */
function stringVectorOfRankedFeature(theVectorOfRankedFeature) {

    if (theVectorOfRankedFeature == null) {
        return "";
    }

    var myString = "[";
    for (var i = 0; i < theVectorOfRankedFeature.length; ++i) {
        myString += stringRankedFeature(theVectorOfRankedFeature[i]);
    }
    myString += "]";

    return myString;
}

function parseVectorOfRankedFeature(theVectorOfString) {
    var myVectorOfRankedFeature = [];
    for (var i = 0; i < theVectorOfString.length; ++i) {
        var myRankedFeature = theVectorOfString[i];
        myVectorOfRankedFeature.push(parseRankedFeature(myRankedFeature));
    }
    return myVectorOfRankedFeature;
}

// prepare identifiers for use as attribute values
//
// filters out everything except for 
// whitespace, alphanumeric characters and the dot.
//
// can be used to convert a filename into a ?dom ID?
//
function stripIdentifier(theIdentifier) {
    return theIdentifier.replace(/[^\w\s.]/g, "");
}

function preloadImages() {
    for (var i = 0; i < window.scene.images.childNodes.length; i++) {
        preLoad(window.scene.images.childNode(i));        
    }
}

function preloadTextures() {
    for (var i = 0; i < window.scene.textures.childNodes.length; i++) {
        preLoad(window.scene.textures.childNode(i));   
    }
}

function blurImage(theImageNode, theRadius, theSigma) {
    var mySize = getImageSize(theImageNode);
    if( theSigma == null ) {
        theSigma = 1.0;
    }
    applyImageFilter(theImageNode, "gaussianblur", [theRadius, mySize.x, mySize.y, theSigma]);
}

function getImageSize(theImage) {
    if (!theImage.src && theImage.childNodes.length == 0) {
        Logger.error("### ERROR: src attribute must be set before getting size for image: "+ theImage.id);
        return undefined;
    }
    var mySize = new Vector3f(theImage.width, theImage.height, 0);
    var myImageMatrix = new Matrix4f(theImage.matrix);
    myImageMatrix.setRow(3, [0,0,0,1]);
    mySize = product(mySize, myImageMatrix);
    return new Vector2i(Math.abs(Math.round(mySize.x)), Math.abs(Math.round(mySize.y)));
}
function setupCameraOrtho(theCamera, theWidth, theHeight, theCameraZ, theFarPlaneDistance) {
    var myCameraZ = 500;
    if(theCameraZ != undefined) {
       myCameraZ = theCameraZ;
    }
    var myFarPlaneDistance = myCameraZ*10;
    if(theFarPlaneDistance != undefined) {
       myFarPlaneDistance = theFarPlaneDistance;
    }
    
    theCamera.frustum = new Frustum();
    theCamera.frustum.width = theWidth;
    theCamera.frustum.height = theHeight;
    theCamera.frustum.type = ProjectionType.orthonormal;
    theCamera.position.x = theWidth/2;
    theCamera.position.y = theHeight/2;
    theCamera.position.z = myCameraZ;
    theCamera.frustum.near = 0.1;
    theCamera.frustum.far  = myFarPlaneDistance; 
    theCamera.orientation = Quaternionf.createFromEuler(new Vector3f(0,0,0));
}