/*jslint nomen:false plusplus:false*/
/*globals Vector4f, Vector2i, Exception, Vector2f, Logger*/

function ShapeStretcher(theShape, theOptions) {
    this.Constructor(this, theShape, theOptions);
}

/////////////
// Statics //
/////////////

ShapeStretcher.DEFAULT_QUADS_PER_SIDE = new Vector2i(3, 3);
ShapeStretcher.APPLY_EDGE_FILTERING_OFFSET_FLAG = true;

ShapeStretcher.applyEdgeFilteringOffset = function (theEdgeAmount) {
    return (!ShapeStretcher.APPLY_EDGE_FILTERING_OFFSET_FLAG || theEdgeAmount < 0) ? 0 : 1;
};

ShapeStretcher.prototype.Constructor = function (Public, theShape, theOptions) {
    
    ////////////////
    // Validation //
    ////////////////
    
    if (!theShape) {
        throw new Exception("Shape must be provided");
    }
    
    /////////////////////
    // Private Members //
    /////////////////////
    
    var _myShape      = theShape;
    var _myVertices   = _myShape.find(".//*[@name='position']").firstChild.nodeValue;
    var _myUVCoords   = null;
    
    var _myEdges = null;
    if (theOptions && "edges" in theOptions) {
        _myEdges = theOptions.edges;
    } else {
        _myEdges = {'top'    : 0,
                    'left'   : 0,
                    'bottom' : 0,
                    'right'  : 0};
    }
    
    var _myCrop  = null;
    if (theOptions && "crop" in theOptions) {
        _myCrop = theOptions.crop;
    } else {
        _myCrop = {'top'    : 0,
                   'left'   : 0,
                   'bottom' : 0,
                   'right'  : 0};
    }
    
    var _myQuadsPerSide = null;
    if (theOptions && "quadsPerSide" in theOptions) {
        // TODO check that quadsPerSide are odd. Even quadsPerSide is possible but
        // it is not entirely clear from which "direction" the stretching is applied
        _myQuadsPerSide = theOptions.quadsPerSide;
    } else {
        _myQuadsPerSide = ShapeStretcher.DEFAULT_QUADS_PER_SIDE.clone();
    }

    var _myVerticesPerSide = new Vector2i(_myQuadsPerSide.x + 1,
                                          _myQuadsPerSide.y + 1);
    
    var _myNumVertices     = _myVerticesPerSide.x * _myVerticesPerSide.y;
    var _myNumQuads        = _myQuadsPerSide.x * _myQuadsPerSide.y;
    
    /////////////////////
    // Private Methods //
    /////////////////////
    
    function _getVertexData(theShape, theVertexAttribute) {
        var myXPath = ".//*[@name='" + theVertexAttribute + "']";
        return theShape.find(myXPath).firstChild.nodeValue;
    }

    function _getIndexData(theShape, theVertexAttribute) {
        var myXPath = ".//*[@vertexdata='" + theVertexAttribute + "']";
        return theShape.find(myXPath).firstChild.nodeValue;
    }
    
    ////////////////////
    // Public Methods //
    ////////////////////
    
    Public.updateGeometry = function (theSize, theUVCoordFlag, theOrigin) {
        var myWidth  = theSize.x;
        var myHeight = theSize.y;
        var myOrigin = theOrigin;
        var v = 0;
        
        for (var i = 0; i < _myVerticesPerSide.y; ++i) {
            for (var j = 0; j < _myVerticesPerSide.x; ++j) {
                v = i * _myVerticesPerSide.x + j;
                var myX = -myOrigin.x;
                var myY = -myOrigin.y;
                var myCropX = 0;
                var myCropY = 0;
                if (j === 0) {
                    myX = -myOrigin.x;
                    myCropX = _myCrop.left;
                } else if (j === _myVerticesPerSide.x - 3) {
                    myX = clamp(-myOrigin.x + _myEdges.left +
                          ShapeStretcher.applyEdgeFilteringOffset(_myEdges.left),0,myWidth);
                    myCropX = _myCrop.left;
                } else if (j === _myVerticesPerSide.x - 2) {
                    myX = clamp(myWidth - myOrigin.x - _myEdges.right -
                          ShapeStretcher.applyEdgeFilteringOffset(_myEdges.right),Math.min(_myEdges.left,myWidth),myWidth);
                    myCropX = -_myCrop.right;
                } else if (j === _myVerticesPerSide.x - 1) {
                    myX = clamp(myWidth - myOrigin.x,Math.min(_myEdges.left,myWidth),myWidth);
                    myCropX = -_myCrop.right;
                }
                if (i === 0) {
                    myY = -myOrigin.y;
                    myCropY = _myCrop.bottom;
                } else if (i === _myVerticesPerSide.y - 3) {
                    myY = clamp(-myOrigin.y + _myEdges.bottom +
                          ShapeStretcher.applyEdgeFilteringOffset(_myEdges.bottom),0,myHeight);
                    myCropY = _myCrop.bottom;
                } else if (i === _myVerticesPerSide.y - 2) {
                    myY = clamp(myHeight - myOrigin.y - _myEdges.top -
                          ShapeStretcher.applyEdgeFilteringOffset(_myEdges.top),Math.min(_myEdges.bottom,myHeight),myHeight);
                    myCropY = -_myCrop.top;
                } else if (i === _myVerticesPerSide.y - 1) {
                    myY = clamp(myHeight - myOrigin.y,Math.min(_myEdges.bottom,myHeight),myHeight);
                    myCropY = -_myCrop.top;
                }
                _myVertices[v] = [myX, myY, 0];
                if (theUVCoordFlag) {
                    _myUVCoords[v] = [(myWidth > 0 ? (myX + myCropX + myOrigin.x) / myWidth : 0),
                                      (myHeight > 0 ? 1 - (myY + myCropY + myOrigin.y) / myHeight : 0)];
                }
            }
        }
    };
    
    Public.setupGeometry = function (theSize, theOrigin) {
        _myUVCoords.resize(_myNumVertices);
        _myVertices.resize(_myNumVertices);
        Public.updateGeometry(theSize, true, theOrigin);
        
        var myPIdx = _getIndexData(_myShape, 'position');
        myPIdx.resize(_myNumQuads * 4);
        var myUVIdx = _getIndexData(_myShape, 'uvset');
        myUVIdx.resize(_myNumQuads * 4);
        var v = 0;
        for (var currentY = 0; currentY < _myQuadsPerSide.y; ++currentY) {
            for (var currentX = 0; currentX < _myQuadsPerSide.x; ++currentX) {
                v = currentY * _myVerticesPerSide.x + currentX;
                var q = 4 * (currentY * _myQuadsPerSide.x + currentX);
                myUVIdx[q]     = myPIdx[q]     = v;
                myUVIdx[q + 1] = myPIdx[q + 1] = v + 1;
                myUVIdx[q + 2] = myPIdx[q + 2] = v + 1 + _myVerticesPerSide.x;
                myUVIdx[q + 3] = myPIdx[q + 3] = v + _myVerticesPerSide.x;
            }
        }
    };
    
    Public.initialize = function () {
        _myUVCoords        = _getVertexData(_myShape, 'uvset');
        _myVerticesPerSide = new Vector2f(_myQuadsPerSide.x + 1,
                                          _myQuadsPerSide.y + 1);
        _myNumVertices     = _myVerticesPerSide.x * _myVerticesPerSide.y;
        _myNumQuads        = _myQuadsPerSide.x    * _myQuadsPerSide.y;
        Logger.debug("ShapeStretcher::  quadsPerSide " + _myQuadsPerSide +
                     ", verticesPerSide " + _myVerticesPerSide +
                     ", numVertices " + _myNumVertices +
                     ", numQuads " + _myNumQuads);
    };
    
    // TODO potentially make edges simply public
    Public.__defineGetter__("edges", function () {
        return _myEdges;
    });
    
    Public.__defineGetter__("crop", function () {
        return _myCrop;
    });
};
