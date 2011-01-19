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
 * Animations that are time-compositions of their children.
 */
GUI.CompositeAnimation = {};

GUI.CompositeAnimation.Constructor = function(Public, Protected) {
    var Base = {};

    GUI.Animation.Constructor(Public, Protected);

    ////////////////////////////////////////
    // Member
    ////////////////////////////////////////

    var _children = [];

    ////////////////////////////////////////
    // Public
    ////////////////////////////////////////

    Public.children getter = function() {
        return _children;
    };

    // add a child, also updating duration
    Public.add = function(theAnimation) {
        _children.push(theAnimation);
        theAnimation.parent = Public;
        Public.childDurationChanged();
    };

    // duration computation, should be overridden
    Public.childDurationChanged = function(theChild) {
    };

    // generic cancel
    Base.cancel = Public.cancel;
    Public.cancel = function() {
        for(var i = 0; i < _children.length; i++) {
            if(_children[i].running) {
                _children[i].cancel();
            }
        }
        Base.cancel();
    };

    // generic finish
    Base.finish = Public.finish;
    Public.finish = function(theComeToAnEndFlag){
        if (!theComeToAnEndFlag) {
            for(var i = 0; i < _children.length; i++) {
                if(_children[i].running) {
                    _children[i].finish();
                }
            }
        }
        Base.finish();
    };

    // generic comeToAnEnd
    Base.comeToAnEnd = Public.comeToAnEnd;
    Public.comeToAnEnd = function(){
        for(var i = 0; i < _children.length; i++) {
            _children[i].comeToAnEnd();
        }
        Base.comeToAnEnd();
    };
};
