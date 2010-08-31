
spark.Rectangle = spark.ComponentClass("Rectangle");

spark.Rectangle.Constructor = function(Protected) {
    var Base = {};
    var Public = this;

    this.Inherit(spark.ResizableRectangle);

    var _myColor;
    Base.realize = Public.realize;
    Public.realize = function(theMaterialOrShape) {
        var myMaterial = null;
        if(theMaterialOrShape) {
            if (theMaterialOrShape.nodeName == "shape") {
                var myShape = theMaterialOrShape;
                var myMateriaId = _myShape
                    . childNode("primitives")
                    . childNode("elements").material;
                myMaterial = _myShape.getElementById(myMateriaId);
            } else if (theMaterialOrShape.nodeName == "material") {
                myMaterial = theMaterialOrShape;
            }
        } else {
            _myColor = Protected.getVector3f("color", null);
            _myColor = _myColor || Protected.getVector4f("color", [1,1,1,1]);
            
            if(_myColor.length < 4) {
                _myColor = new Vector4f(_myColor.x, _myColor.y, _myColor.z, Public.alpha);
            } 
            
            myMaterial = Modelling.createColorMaterial(window.scene, _myColor);
            
            myMaterial.transparent = true;
        }
        Base.realize(myMaterial);
    };
    
    //XXX without this transparency is not working??
    Base.postRealize = Public.postRealize;
    Public.postRealize = function(){
        Base.postRealize();
        if(_myColor) {
            Public.color = _myColor;
        }
    };

    Public.color getter = function() {
        var c = Protected.material.properties.surfacecolor;
        return new Vector4f(c[0], c[1], c[2], c[3]);
    };

    Public.color setter = function(theColor) {
        Protected.material.properties.surfacecolor = theColor;
    };
}
