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

#ifndef _IJEXTENSION_INCLUDED
#define _IJEXTENSION_INCLUDED

#include "y60_jslib_settings.h"

#include <asl/base/Ptr.h>
#include <y60/jsbase/PropertyValue.h>

struct JSContext;
struct JSObject;

namespace jslib {

struct IScriptablePlugin {
    virtual ~IScriptablePlugin() {}
    virtual void initClasses(JSContext * theContext, JSObject *theGlobalObject) = 0;

    // Function will be called if a property is read in js
    virtual void onGetProperty(const std::string & thePropertyName,
        y60::PropertyValue & theReturnValue) const
    {}

    // Function will be called if a property is set in js
    virtual void onSetProperty(const std::string & thePropertyName,
        const y60::PropertyValue & thePropertyValue)
    {}

    // Function will be called if a setting is changed (s. BaseConfigurator.js)
    virtual void onUpdateSettings(dom::NodePtr theConfiguration) {};
};

typedef asl::Ptr<IScriptablePlugin> IScriptablePluginPtr;

}
#endif
