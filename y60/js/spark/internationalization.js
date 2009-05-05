/* __ ___ ____ _____ ______ _______ ________ _______ ______ _____ ____ ___ __
//
// Copyright (C) 1993-2009, ART+COM AG Berlin, Germany <www.artcom.de>
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

spark.I18nContext = spark.ComponentClass("I18nContext");

spark.I18nContext.Constructor = function(Protected) {
    var Public = this;
    var Base = {};

    this.Inherit(spark.Container);
    this.Inherit(spark.EventDispatcher);

    var _myLanguage = "";

    Public.language getter = function() {
        return _myLanguage;
    };

    Public.switchLanguage = function(theLanguage) {
        if(theLanguage == _myLanguage) {
            return;
        }

        var myContextEvent = new spark.I18nEvent(theLanguage);
        Public.dispatchEvent(myContextEvent);

        var myChildren = Public.children;
        for(var i = 0; i < myChildren.length; i++) {
            var myChild = myChildren[i];
            myChild.switchLanguage(theLanguage);
        }

        _myLanguage = theLanguage;
    };
    
};


spark.I18nEvent = spark.Class("I18nEvent");

spark.I18nEvent.LANGUAGE = "language";

spark.I18nEvent.Constructor = function(Protected, theLanguage) {
    var Public = this;
    var Base = {};

    this.Inherit(spark.Event, spark.I18nEvent.LANGUAGE);

    var _myLanguage = theLanguage;
    
    Public.language getter = function() {
        return _myLanguage;
    };

    var _myData = null;

    Public.data getter = function() {
        return _myData;
    };

    Public.data setter = function(theValue) {
        _myData = theValue;
    };

};


spark.I18nItem = spark.AbstractClass("I18nItem");

spark.I18nItem.Constructor = function(Protected) {
    var Public = this;
    var Base = {};

    this.Inherit(spark.Component);
    this.Inherit(spark.EventDispatcher);

    var _myLanguage = "";

    var _myLanguageNodes = {};

    Protected.languageNodes = function() {
        return _myLanguageNodes;
    };

    Base.realize = Public.realize;
    Public.realize = function() {
        var myNode = Public.node;
        for(var i = 0; i < myNode.childNodesLength(); i++) {
            var myChild = myNode.childNode(i);
            _myLanguageNodes[myChild.nodeName] = myChild;
        }
    };

    Protected.createEvent = function(theLanguage) {
        return new spark.I18nEvent(theLanguage);
    };

    Protected.getNode = function(theLanguage) {
        if(!(theLanguage in _myLanguageNodes)) {
            throw new Error("I18n item " + Public.name + " does not contain language " + theLanguage);
        } else {
            return _myLanguageNodes[theLanguage];
        }
    };

    Public.switchLanguage = function(theLanguage) {
        if(theLanguage == _myLanguage) {
            return;
        }

        if(!(theLanguage in _myLanguageNodes)) {
            throw new Error("I18n item " + Public.name + " does not contain language " + theLanguage);
        }
        
        var myEvent = Protected.createEvent(theLanguage);
        Public.dispatchEvent(myEvent);

        _myLanguage = theLanguage;
    };
    
};

spark.I18nText = spark.ComponentClass("I18nText");

spark.I18nText.Constructor = function(Protected) {
    var Public = this;
    var Base = {};

    this.Inherit(spark.I18nItem);

    Base.realize = Public.realize;
    Public.realize = function() {
        Base.realize();
    };

    Base.createEvent = Protected.createEvent;
    Protected.createEvent = function(theLanguage) {
        var myNode = Protected.getNode(theLanguage);
        var myEvent = Base.createEvent(theLanguage);
        myEvent.data = myNode.childNode("#text").nodeValue;
        return myEvent;
    };
};

spark.I18nImage = spark.ComponentClass("I18nImage");

spark.I18nImage.Constructor = function(Protected) {
    var Public = this;
    var Base = {};

    this.Inherit(spark.I18nItem);

    Base.realize = Public.realize;
    Public.realize = function() {
        Base.realize();
    };

    Base.createEvent = Protected.createEvent;
    Protected.createEvent = function(theLanguage) {
        var myNode = Protected.getNode(theLanguage);
        var myEvent = Base.createEvent(theLanguage);
        var mySource = myNode.childNode("#text").nodeValue;
        myEvent.data = spark.getCachedImage(mySource);
        return myEvent;
    };
};
