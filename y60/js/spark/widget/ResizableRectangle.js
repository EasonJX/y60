spark.ResizableRectangle = spark.AbstractClass("ResizableRectangle");

spark.ResizableRectangle.Constructor = function(Protected) {
    var Base = {};
    var Public = this;

    this.Inherit(spark.Body);

    var _myWidth;
    var _myHeight;
    var _myMaterial;
    var _myShape;
    var _myVertices;

    Base.realize = Public.realize;
    Public.realize = function(theMaterialOrImageOrShape) {
        if (theMaterialOrImageOrShape.nodeName == "shape") {
            _myShape = theMaterialOrImageOrShape;
            var myMateriaId = _myShape
                . childNode("primitives")
                . childNode("elements").material;
            _myMaterial = _myShape.getElementById(myMateriaId);
        } else if (theMaterialOrImageOrShape.nodeName == "material") {
            _myMaterial = theMaterialOrImageOrShape;
        }

        if ( ! _myShape) {
            var tu = _myMaterial.find("./textureunits/textureunit");
            if (tu) {
               var raster = tu.$texture.$image.raster;
               _myWidth  = Protected.getNumber("width", raster.width);
               _myHeight = Protected.getNumber("height", raster.height);
            } else {
               _myWidth  = Protected.getNumber("width", 1);
               _myHeight = Protected.getNumber("height", 1);
            }

            var mySize = new Vector3f(_myWidth, _myHeight, 0);
                               
            Public.origin = Protected.getVector3f("origin", Public.origin);
            var myLowerLeft = product( Public.origin, -1);
            var myUpperRight = difference( mySize, Public.origin);

            _myShape = Modelling.createQuad(window.scene, _myMaterial.id,
                    myLowerLeft, myUpperRight);
            _myShape.name = Public.name + "-shape";
        }

        _myVertices = _myShape.find(".//*[@name='position']").firstChild.nodeValue;
        
        var myBody  = Modelling.createBody(Public.parent.innerSceneNode, _myShape.id);
        if(Public.name) {
            myBody.name = Public.name;
        } else {
            myBody.name = "unnamed-spark-body";
        }
	
        Base.realize(myBody);        
    };

    // XXX untested with non-zero origin
    Public.width getter = function() { return _myWidth; }
    Public.width setter = function(w) {
        if (w != _myWidth) {
            var o = Public.origin;
            _myVertices[1] = [w - o.x, -o.y, -o.z];
            _myVertices[3] = [w - o.x, _myHeight - o.y, o.z];
            _myWidth = w;
        }
    }
    
    Public.height getter = function() { return _myHeight; }
    Public.height setter = function(h) {
        if (h != _myHeight) {
            var o = Public.origin;
            _myVertices[2] = [-o.x, h - o.y, -o.z];
            _myVertices[3] = [_myWidth - o.x, h - o.y, -o.z];
            _myHeight = h;
        }
    }
    
    Public.size getter = function() { return new Vector2f(_myWidth, _myHeight);}
    Public.size setter = function(s) {
        Public.width = s.x;
        Public.height = s.y;
    }
    
    Protected.material getter = function() { return _myMaterial; }
    Protected.shape    getter = function() { return _myShape; }
    Protected.vertices getter = function() { return _myVertices; }
}
