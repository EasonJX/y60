//=============================================================================
// Copyright (C) 2004-2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

//use("ShapeBuilder.js"); // UH: self-referential reference? ;-)

const ELEMENT_TYPE_POINTS          = "points";
const ELEMENT_TYPE_LINES           = "lines";
const ELEMENT_TYPE_LINE_STRIP      = "linestrip";
const ELEMENT_TYPE_LINE_LOOP       = "lineloop";
const ELEMENT_TYPE_TRIANGLES       = "triangles";
const ELEMENT_TYPE_TRIANGLE_STRIP  = "trianglestrip";
const ELEMENT_TYPE_TRIANGLE_FAN    = "trianglefan";
const ELEMENT_TYPE_QUADS           = "quads";
const ELEMENT_TYPE_QUAD_STRIP      = "quadstrip";
const ELEMENT_TYPE_POLYGON         = "polygon";

function Element(theType, theMaterialId) {
    this.Constructor(this, theType, theMaterialId);
}

Element.prototype.Constructor = function(self, theType, theMaterialId) {

    self.type = theType;
    self.material = theMaterialId;

    // position,normal,etc. indices
    self.positions = [];
    self.normals   = [];
    self.colors    = [];
    self.texcoords = [];
}

/**
 */
function ShapeBuilder() {
    this.Constructor(this);
}

ShapeBuilder.prototype.Constructor = function(obj) {

    var _myElements  = [];

    // position,normal,etc. data
    var _myPositions = [];
    var _myNormals   = [];
    var _myColors    = [];
    var _myUVCoords  = [];

    /// Create <shape> node from the data previously passed in.
    obj.buildNode = function(theName) {

        if (theName == null) {
            theName = "ShapeBuilder";
        }

        var myShapeString =
            '<shape name="' + theName + '" renderstyle="[frontfacing]">\n' +
            ' <vertexdata>\n' +
            '  <vectorofvector3f name="position">' + arrayToString(_myPositions) + '</vectorofvector3f>\n';
        if (_myNormals.length > 0) {
            myShapeString += '  <vectorofvector3f name="normal">' +  arrayToString(_myNormals) + '</vectorofvector3f>\n';
        }
        if (_myColors.length > 0) {
            myShapeString += '  <vectorofvector4f name="color">' + arrayToString(_myColors) + '</vectorofvector4f>\n';
        }
        if (_myUVCoords.length >0) {
            myShapeString += '  <vectorofvector2f name="map1">' + arrayToString(_myUVCoords) + '</vectorofvector2f>\n';
        }
        myShapeString += ' </vertexdata>\n<primitives>\n';

        for (var i = 0; i < _myElements.length; ++i) {
            var myElement = _myElements[i];
            myShapeString +=
                '  <elements type="' + myElement.type + '" material="' + myElement.material + '">\n' +
                '   <indices vertexdata="position" role="position">' + arrayToString(myElement.positions) + '</indices>\n';
            if (myElement.normals.length > 0) {
                myShapeString += '   <indices vertexdata="normal" role="normal">' + arrayToString(myElement.normals) + '</indices>\n';
            }
            if (myElement.colors.length > 0) {
                myShapeString +='   <indices vertexdata="color" role="color">' + arrayToString(myElement.colors) + '</indices>\n';
            }
            if (myElement.texcoords.length > 0) {
                myShapeString +='   <indices vertexdata="map1" role="texcoord0">' + arrayToString(myElement.texcoords) + '</indices>\n';
            }
            myShapeString +='  </elements>\n';
        }
        myShapeString += '</primitives>\n</shape>\n';

        var myShapeDoc = new Node(myShapeString);
        return myShapeDoc.firstChild;
    }

    /// Append a new shape element of the type using the material.
    obj.appendElement = function(theType, theMaterialId) {
        var e = new Element(theType, theMaterialId);
        _myElements.push(e);
        return e;
    }

    obj.appendVertex = function(theElement, theVertex) {
        var myIndex = _myPositions.length;
        _myPositions.push(theVertex);
        theElement.positions.push(myIndex);
        return myIndex;
    }

    obj.appendNormal = function(theElement, theNormal) {
        var myIndex = _myNormals.length;
        _myNormals.push(theNormal);
        theElement.normals.push(myIndex);
        return myIndex;
    }

    obj.appendColor = function(theElement, theColor) {
        var myIndex = _myColors.length;
        _myColors.push(theColor);
        theElement.colors.push(myIndex);
        return myIndex;
    }

    obj.appendTexCoord = function(theElement, theTexCoord) {
        var myIndex = _myUVCoords.length;
        _myUVCoords.push(theTexCoord);
        theElement.texcoords.push(myIndex);
        return myIndex;
    }

    /// Append a line to the given element.
    obj.appendLineElement = function (theElement, theLineBegin, theLineEnd){
        _myPositions.push(theLineBegin);
        theElement.positions.push(_myPositions.length-1);
        _myPositions.push(theLineEnd);
        theElement.positions.push(_myPositions.length-1);
        theElement.normals.push(0);
        theElement.normals.push(0);
        theElement.colors.push(_myColors.length-1);
        theElement.colors.push(_myColors.length-1);
    }

    /// Append a point to a linestrip element.
    obj.appendLineStripElement = function (theElement, thePosition) {
        Logger.warning("Consider using separate appendVertex");
        theElement.positions.push(obj.appendVertex(thePosition));
        theElement.normals.push(0);
        theElement.colors.push(0);
    }

    obj.appendQuad = function (theElement, thePosition, theSize, theDepth) {

        if (theDepth == null) {
            theDepth = 0;
        }

        // add positions to shape's vertex data
        var myHalfWidth  = theSize[0] / 2;
        var myHalfHeight = theSize[1] / 2;
        var myPosIndex = _myPositions.length;
        _myPositions.push([thePosition[0] - myHalfWidth, thePosition[1] - myHalfHeight, theDepth]);
        _myPositions.push([thePosition[0] + myHalfWidth, thePosition[1] - myHalfHeight, theDepth]);
        _myPositions.push([thePosition[0] + myHalfWidth, thePosition[1] + myHalfHeight, theDepth]);
        _myPositions.push([thePosition[0] - myHalfWidth, thePosition[1] + myHalfHeight, theDepth]);
        // set element's position indices
        theElement.positions.push(myPosIndex,myPosIndex+1,myPosIndex+2,myPosIndex+3);

        // a single normal pointing z for all vertices
        var myNormalIndex = _myNormals.length
        _myNormals.push([0, 0, 1]);
        theElement.normals.push(myNormalIndex,myNormalIndex,myNormalIndex,myNormalIndex);

        // uv coordintaes for the 4 vertices
        var myTexIndex = _myUVCoords.length;
        _myUVCoords.push([0,1]);
        _myUVCoords.push([1,1]);
        _myUVCoords.push([1,0]);
        _myUVCoords.push([0,0]);
        theElement.texcoords.push(myTexIndex,myTexIndex+1,myTexIndex+2,myTexIndex+3);
    }

    obj.appendQuad2 = function (theElement, p1, p2, p3, p4) {
        // add positions to shape's vertex data
        var myPosIndex = _myPositions.length

        _myPositions.push(p1);
        _myPositions.push(p2);
        _myPositions.push(p3);
        _myPositions.push(p4);

        // set element's position indices
        theElement.positions.push(myPosIndex,myPosIndex+1,myPosIndex+2,myPosIndex+3);

        // a single normal pointing z for all vertices
        var myNormalIndex = _myNormals.length
        _myNormals.push([0, 0, 1]);
        theElement.normals.push(myNormalIndex,myNormalIndex,myNormalIndex,myNormalIndex);

        // uv coordintaes for the 4 vertices
        var myTexIndex = _myUVCoords.length;
        _myUVCoords.push([0,1]);
        _myUVCoords.push([1,1]);
        _myUVCoords.push([1,0]);
        _myUVCoords.push([0,0]);
        theElement.texcoords.push(myTexIndex,myTexIndex+1,myTexIndex+2,myTexIndex+3);
    }

    obj.appendQuadWithCustomTexCoords = function (theElement, thePosition, theSize, theUVOrigin, theUVSize, theGenSingleVertexColorFlag) {
        // add positions to shape's vertex data
        var myHalfWidth  = theSize[0] / 2;
        var myHalfHeight = theSize[1] / 2;
        var myPosIndex = _myPositions.length;
        _myPositions.push([thePosition[0] - myHalfWidth, thePosition[1] - myHalfHeight, 0]);
        _myPositions.push([thePosition[0] + myHalfWidth, thePosition[1] - myHalfHeight, 0]);
        _myPositions.push([thePosition[0] + myHalfWidth, thePosition[1] + myHalfHeight, 0]);
        _myPositions.push([thePosition[0] - myHalfWidth, thePosition[1] + myHalfHeight, 0]);
        // set element's position indices
        theElement.positions.push(myPosIndex,myPosIndex+1,myPosIndex+2,myPosIndex+3);

        // a single normal pointing z for all vertices
        var myNormalIndex = _myNormals.length
        _myNormals.push([0, 0, 1]);
        theElement.normals.push(myNormalIndex,myNormalIndex,myNormalIndex,myNormalIndex);

        // uv coordintaes for the 4 vertices
        var myTexIndex = _myUVCoords.length;

        _myUVCoords.push([theUVOrigin[0], theUVOrigin[1] + theUVSize[1]]);
        _myUVCoords.push([theUVOrigin[0] + theUVSize[0], theUVOrigin[1] + theUVSize[1]]);
        _myUVCoords.push([theUVOrigin[0] + theUVSize[0], theUVOrigin[1]]);
        _myUVCoords.push([theUVOrigin[0], theUVOrigin[1]]);

        theElement.texcoords.push(myTexIndex,myTexIndex+1,myTexIndex+2,myTexIndex+3);

        var myColorIndex = _myColors.length;
        if (theGenSingleVertexColorFlag || theGenSingleVertexColorFlag == undefined) {
            _myColors.push([1,1,1,1]);        
            theElement.colors.push(myColorIndex, myColorIndex, myColorIndex, myColorIndex);
        } else {
            _myColors.push([1,1,1,1]);        
            _myColors.push([1,1,1,1]);        
            _myColors.push([1,1,1,1]);        
            _myColors.push([1,1,1,1]);        
            theElement.colors.push(myColorIndex, myColorIndex+1, myColorIndex+2, myColorIndex+3);
        }
    }
}
