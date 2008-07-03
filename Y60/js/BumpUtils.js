//=============================================================================
// Copyright (C) 2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

use("Y60JSSL.js");


function calculateTangent(P0, P1, P2, UV0, UV1, UV2) {

    var V10 = difference(P1, P0);
    var V20 = difference(P2, P0);
    var T10 = difference(UV1, UV0);
    var T20 = difference(UV2, UV0);

    // 2x2 matrix inverse
    // http://de.wikipedia.org/wiki/Regul%C3%A4re_Matrix
    var d = 1.0 / (T10.x * T20.y - T10.y * T20.x);
    var Minv = [ new Vector2f(d * T20.y, d * -T10.y), new Vector2f(d*-T20.x, d*T10.x) ];

    // tangent is uniform per primitive
    var T = sum(product(V10, Minv[0][0]), product(V20, Minv[0][1]));
    var B = sum(product(V10, Minv[1][0]), product(V20, Minv[1][1]));
    //print(T10, T20, T, B);

    return T;
}


function calculateAndAppendTangents(theShape) {

    var myElements = theShape.childNode("primitives").firstChild;
    var myVertexData = theShape.childNode("vertexdata");

    var myVertexIndexNode = getDescendantByAttribute(myElements, "role", "position");
    var myVertexIndex = myVertexIndexNode.firstChild.nodeValue;
    var myVertices = getDescendantByName(myVertexData, myVertexIndexNode.vertexdata).firstChild.nodeValue;

    var myTexCoordIndexNode = getDescendantByAttribute(myElements, "role", "texcoord0");
    var myTexCoordIndex = myTexCoordIndexNode.firstChild.nodeValue;
    var myTexCoords = getDescendantByName(myVertexData, myTexCoordIndexNode.vertexdata).firstChild.nodeValue;

    // tangent indices
    var myTangentIndex = [];
    var myTangentData = [];

    var myNumVertices = 3;
    if (myElements.type == "quads" || myElements.type == "quadstrip") {
        myNumVertices = 4;
    }

    //print("vertex", myVertices.length, myVertexIndex.length);
    //print("texcoord", myTexCoords.length, myTexCoordIndex.length);
    for (var i = 0; i < myVertexIndex.length; i += myNumVertices) {

        var P0 = myVertices[myVertexIndex[i]];
        var P1 = myVertices[myVertexIndex[i+1]];
        var P2 = myVertices[myVertexIndex[i+2]];

        var UV0 = myTexCoords[myTexCoordIndex[i]];
        var UV1 = myTexCoords[myTexCoordIndex[i+1]];
        var UV2 = myTexCoords[myTexCoordIndex[i+2]];

        var T = calculateTangent(P0,P1,P2, UV0,UV1,UV2);

        var TI = myTangentData.length;
        myTangentData.push(normalized(T));
        for (var j = 0; j < myNumVertices; ++j) {
            myTangentIndex.push(TI);
        }
    }
    //print("tangent", myTangentData.length, myTangentIndex.length);

    // append tangent data
    var myTangentIndexNode = new Node("<indices vertexdata='tangent' role='texcoord1'>["+myTangentIndex+"]</indices>").firstChild;
    myElements.appendChild(myTangentIndexNode);

    var myTangentDataNode = new Node("<vectorofvector3f name='tangent'>["+myTangentData+"]</vectorofvector3f>").firstChild;
    myVertexData.appendChild(myTangentDataNode);
}


// create image node if theImage is a filename
function ensureImageNode(theScene, theImage)
{
    var myImage = null;
    if (typeof(theImage) == "string") {
        myImage = Modelling.createImage(theScene, theImage);
        myImage.id = createUniqueId();
    } else {
        myImage = theImage;
    }

    return myImage;
}

// get or create texture that references theImage
function getOrCreateTexture(theScene, theImage)
{
    var myImage = ensureImageNode(theScene, theImage);

    var myTexture = getDescendantByAttribute(theScene.textures, "image", myImage.id, true);
    if (!myTexture) {
        myTexture = Node.createElement("texture");
        myTexture.id = createUniqueId();
        myTexture.image = myImage.id;
        myTexture.mipmap = 1;
        theScene.textures.appendChild(myTexture);
    }

    return myTexture;
}


function setupBumpMap(theScene, theBodyOrShape, theNormalMap, theBaseMap) {

    var myShape = null;
    if (typeof(theBodyOrShape) != "object") {
        Logger.error("setupBumpMap: theBodyOrShape must be an object");
        return;
    }
    if ("shape" in theBodyOrShape) {
        myShape = theBodyOrShape.getElementById(theBodyOrShape.shape); // body
    } else {
        myShape = theBodyOrShape; // shape
    }

    if (theNormalMap == null) {
        Logger.error("setupBumpMap: theNormalMap must be given");
        return;
    }
    // theBaseMap is optional

    // add tangents to shape
    calculateAndAppendTangents(myShape);

    /*
     * setup material
     */
    var myElements = myShape.childNode("primitives").firstChild;
    var myMaterial = myElements.getElementById(myElements.material);

    // tweak requirements for normalmapping
    myMaterial.enabled = false;
    myMaterial.requires.lighting = "[100[phong]]";
    if (theBaseMap) {
        myMaterial.requires.textures = "[100[paint,bump]]";
    } else {
        myMaterial.requires.textures = "[100[bump]]";
    }
    myMaterial.requires.texcoord = "[100[uv_map,uv_map]]";

    var myTextureUnits = myMaterial.childNode("textureunits");
    if (!myTextureUnits) {
        myMaterial.appendChild(Node.createElement("textureunits"));
    }
    myMaterial.enabled = true;

    // add base map
    if (theBaseMap) {
        var myBaseTexture = getOrCreateTexture(theScene, theBaseMap);
        if (myTextureUnits.childNodesLength() == 0) {
            var myTextureUnit = Node.createElement("textureunit");
            myTextureUnit.texture = myBaseTexture.id;
            myTextureUnits.appendChild(myTextureUnit);
        } else {
            myTextureUnits.firstChild.texture = myBaseTexture.id;
        }
    }

    // add normal map
    if (theNormalMap) {
        var myNormalTexture = getOrCreateTexture(theScene, theNormalMap);
        if (theBaseMap || myTextureUnits.childNodesLength() == 0) {
            var myTextureUnit = Node.createElement("textureunit");
            myTextureUnit.texture = myNormalTexture.id;
            myTextureUnits.appendChild(myTextureUnit);
        } else {
            myTextureUnits.lastChild.texture = myNormalTexture.id;
        }
    }
}
