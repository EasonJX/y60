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

/**
 * Rotate an object using degrees as unit, normalizing the set angles.
 */
GUI.QuaternionAnimation = function(theDuration, theEasing, theObject, theProperty, theStart, theEnd) {
    this.Constructor(this, {}, theDuration, theEasing, theObject, theProperty, theStart, theEnd);
}

GUI.QuaternionAnimation.prototype.Constructor = function(Public, Protected, theDuration, theEasing,
                                                       theObject, theProperty, theStart, theEnd) {
    var Base = {};

    GUI.SimpleAnimation.Constructor.call(Public, Public, Protected);

    ////////////////////////////////////////
    // Member
    ////////////////////////////////////////

    var _object = theObject;
    var _property = "";
    var _start = 0;
    var _end = Quaternionf.createFromEuler(new Vector3f(0,0,0));

    Public.__defineGetter__("object", function() {
        return _object;
    });

    Public.__defineGetter__("property", function() {
        return _property;
    });

    Public.__defineGetter__("start", function() {
        return _start;
    });

    Public.__defineGetter__("end", function() {
        return _end;
    });

    ////////////////////////////////////////
    // Public
    ////////////////////////////////////////

    // initialize from arguments
    Public.setup = function() {
        Protected.duration = theDuration;

        if(theEasing != null) {
            Public.easing = theEasing;
        }

        _object = theObject;
        _property = theProperty;
        _start = new Quaternionf(theStart); // take a copy, just in case the user
                                            // supplied a refernce? [DS]
        _end = theEnd;
    };

    // set the current value
    Base.render = Public.render;
    Public.render = function() {
        var myValue = Quaternionf.slerp(_start, _end, Public.progress);
        _object[_property] = myValue;
    };

    Public.toString = function() {
        return Protected.standardToString("QuaternionAnimation" + " on "  + _object.name + "." + _property);
    };

    Public.setup();
};

