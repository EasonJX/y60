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
 * Play several animations in parallel.
 */
GUI.ParallelAnimation = function() {
    this.Constructor(this, {});
}

GUI.ParallelAnimation.prototype.Constructor = function(Public, Protected) {
    var Base = {};

    GUI.CompositeAnimation.Constructor.call(Public, Public, Protected);

    ////////////////////////////////////////
    // Public
    ////////////////////////////////////////

    // duration = max(map(children, duration))
    Base.childDurationChanged = Public.childDurationChanged;
    Public.childDurationChanged = function(theChild) {
    	var d = 0;
	    for(var i = 0; i < Public.children.length; i++) {
	        var c = Public.children[i].duration;
		    if(c > d) {
				d = c;
			}
		}
	    Protected.duration = d;
	};
	
    // start playing all children
    Base.play = Public.play;
	Public.play = function() {
		Base.play();
		for(var i = 0; i < Public.children.length; i++) {
			Public.children[i].play();
		}
	};
	
    // step all children forward
	Public.doFrame = function()	{
		var finished = true;
		for(var i = 0; i < Public.children.length; i++) {
			Public.children[i].doFrame();
			if(Public.children[i].running) {
				finished = false;
			}
		}
		
		if(finished) {
			Public.finish();
		}
	};
	
	Public.toString = function() {
		return "ParallelAnimation";
	};
	
};
