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
//
//   $Id: LightSourceBuilder.cpp,v 1.1 2005/03/24 23:35:56 christian Exp $
//   $RCSfile: LightSourceBuilder.cpp,v $
//   $Author: christian $
//   $Revision: 1.1 $
//   $Date: 2005/03/24 23:35:56 $
//
//
//  Description: XML-File-Export Plugin for light sources
//
// (CVS log at the bottom of this file)
//
//=============================================================================

#include "LightSourceBuilder.h"
#include "LightSource.h"
#include <y60/base/NodeNames.h>
#include <y60/base/PropertyNames.h>
#include <y60/base/property_functions.h>

//TODO: change all setPropertyValue<> stuff to facade access

namespace y60 {
    LightSourceBuilder::LightSourceBuilder(const std::string & theName) : BuilderBase(LIGHTSOURCE_NODE_NAME) {
        dom::NodePtr myNode = getNode();

        if (!myNode->hasFacade()) {
            myNode->appendAttribute(NAME_ATTRIB, theName);
        } else {
            LightSourcePtr myLightSource = getNode()->getFacade<LightSource>();
            myLightSource->set<NameTag>(theName);
        }
 
        (*myNode)(PROPERTY_LIST_NAME);
    }

    LightSourceBuilder::~LightSourceBuilder() {
    }

    void
    LightSourceBuilder::setSpotLight(const float & theCutoff, const float & theExponent) {
        setPropertyValue<float>(getNode(), "float", SPOTLIGHT_CUTOFF_ATTRIB, theCutoff);
        setPropertyValue<float>(getNode(), "float", SPOTLIGHT_EXPONENT_ATTRIB, theExponent);
        (*getNode())[LIGHTSOURCE_TYPE_ATTRIB] =  SPOT_LIGHT;
    }

    void
    LightSourceBuilder::setAmbient(const asl::Vector4f & theAmbient) {
        setPropertyValue<asl::Vector4f>(getNode(), "vector4f", AMBIENT_PROPERTY, theAmbient);
    }

    void
    LightSourceBuilder::setDiffuse(const asl::Vector4f & theDiffuse) {
        setPropertyValue<asl::Vector4f>(getNode(), "vector4f", DIFFUSE_PROPERTY, theDiffuse);
    }

    void
    LightSourceBuilder::setAttenuation(const float & theAttenuation) {
        setPropertyValue<float>(getNode(), "float", ATTENUATION_PROPERTY, theAttenuation);
    }

    void
    LightSourceBuilder::setSpecular(const asl::Vector4f & theSpecular) {
        setPropertyValue<asl::Vector4f>(getNode(), "vector4f", SPECULAR_PROPERTY, theSpecular);
    }

    void
    LightSourceBuilder::setType(const LightSourceType theLightSourceType) {
        dom::NodePtr myNode = getNode();
        if (!myNode->hasFacade()) {
            myNode->appendAttribute(LIGHTSOURCE_TYPE_ATTRIB, asl::getStringFromEnum(theLightSourceType, LightSourceTypeString));
        } else {
            myNode->getFacade<LightSource>()->set<LightSourceTypeTag>(asl::getStringFromEnum(theLightSourceType, LightSourceTypeString));
        }
    }
}
