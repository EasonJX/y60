//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: LightSource.cpp,v $
//   $Author: pavel $
//   $Revision: 1.8 $
//   $Date: 2005/04/24 00:41:21 $
//
//  Description: This class is the base of all lights.
//
//=============================================================================

#include "LightSource.h"

#include <y60/base/PropertyNames.h>
#include <y60/base/property_functions.h>
#include <asl/base/string_functions.h>

namespace y60 {

    LightSource::LightSource(dom::Node & theNode) : 
        IdTag::Plug(theNode),
        NameTag::Plug(theNode),
		LightPropertiesTag::Plug(this),
        LightSourceTypeTag::Plug(theNode),
        Facade(theNode)
    {}
    
    const LightSourceType 
    LightSource::getType() {
        if (true /* node version changed */) {
            _myType = LightSourceType(asl::getEnumFromString(get<LightSourceTypeTag>(), LightSourceTypeString));
        }
        return _myType;
    }
} 
