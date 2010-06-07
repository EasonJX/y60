/*jslint nomen: false*/
/*globals use, spark, OffscreenRenderArea, Modelling, window, Node, Vector3f*/

/**
 * Simple wrapper for the Y60 offscreen renderer.
 */

use("SceneViewer.js"); // XXX: bad place for this

spark.Canvas = spark.ComponentClass("Canvas");

spark.Canvas.Constructor = function (Protected) {
    var Public = this;
    var Base = {};

    //this.Inherit(spark.Stage);
    this.Inherit(spark.ResizableRectangle);

    var _myRenderArea = null;
    var _myWorld = null;

    Public.innerSceneNode getter = function () {
        return _myWorld;
    };

    Base.realize = this.realize;
    this.realize = function () {
        _myRenderArea = new OffscreenRenderArea();

        var myWidth = Protected.getNumber("width", 100);
        var myHeight = Protected.getNumber("height", 100);
    	var myImage = Modelling.createImage(window.scene,
    										myWidth, myHeight, "BGRA");
    	myImage.name = Public.name + "_canvasImage";
    	var myTexture = Modelling.createTexture(window.scene, myImage);
      myTexture.wrapmode = "clamp_to_edge";

    	var myFlipMatrix = myTexture.matrix;
    	myFlipMatrix.makeScaling(new Vector3f(1,-1,1));
      myFlipMatrix.translate(new Vector3f(0,1,0));

    	var myMaterial = Modelling.createUnlitTexturedMaterial(window.scene, myTexture);
      myMaterial.transparent = true;
      
      //TODO: check if maybe there are other options than scene merging
      var mySceneFile = Protected.getString("sceneFile", "");
      
      if (mySceneFile) {
          Logger.info("Loading spark file " + mySceneFile);
          if (!fileExists(mySceneFile)) {
              throw Error("spark file '" + theFile + "' does not exist.");
          }
          var myDom = new Node();
          myDom.parseFile(mySceneFile);
          
          // get first world, if more world exist we wont be able to handle the second one
          var myWorldId = myDom.find(".//worlds/world").id;
          var myCanvasId = myDom.find(".//canvases/canvas").id;
          
          mergeScenes(window.scene, myDom);
          
          var myCanvas = window.scene.dom.find(".//canvases/canvas[@id='" + myCanvasId + "']");
          myViewport = myCanvas.find(".//viewport");
          myCamera   = window.scene.dom.getElementById(myWorldId).find(".//camera");
          
          _myWorld = window.scene.dom.getElementById(myWorldId);
          var myLightManager = new LightManager(window.scene, _myWorld);
   //     Public.setLightManager(myLightManager);
          myLightManager.setupDefaultLighting(myCanvas);
        
      } else {
          _myWorld = Node.createElement("world");
        	window.scene.worlds.appendChild(_myWorld);
          _myWorld.name = Public.name + "-world";

        	var myCamera = Node.createElement("camera");
        	_myWorld.appendChild(myCamera);

        	spark.setupCameraOrtho(myCamera, myWidth, myHeight);

        	var myCanvas = Node.createElement("canvas");
        	window.scene.canvases.appendChild(myCanvas);
          myCanvas.name = Public.name + "-canvas";
          myCanvas.backgroundcolor[3] = 0.0;
          
        	var myViewport = Node.createElement("viewport");
        	myViewport.camera = myCamera.id;
        	myCanvas.appendChild(myViewport);
          myViewport.name = Public.name + "-viewport";
      }
      
      myCanvas.target = myTexture.id;
      
    	_myRenderArea.setSceneAndCanvas(window.scene, myCanvas);


        Public.parent.stage.addEventListener(spark.StageEvent.FRAME, function (theEvent) {
            _myRenderArea.renderToCanvas();
        });

        Base.realize(myMaterial);

    };
    
    function mergeScenes(theTargetScene, theModelDom) {
        //TODO: what if id already exists??
        //  - preparse scenes, check for equal ids
        //  - when duplicate ids exist rewrite id and references on id
        //      - maybe just for viewport, canvas and camera ids because we need them, the rest would be same as already in scene???
        //  - paths for model resources have to be adjusted, they are relative to sceneFile path
        var mySceneNode = theModelDom.firstChild;
        while (mySceneNode.childNodesLength()) {
            var childNode = mySceneNode.firstChild;
            var receivingNode = theTargetScene.dom.getNodesByTagName(childNode.nodeName)[0];
            while (childNode.childNodesLength()) {
                var childChildNode = childNode.removeChild(childNode.firstChild);
                receivingNode.appendChild(childChildNode);
            }
            mySceneNode.removeChild(mySceneNode.firstChild);
        }
        
    };
    
};