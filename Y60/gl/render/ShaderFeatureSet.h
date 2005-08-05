//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: ShaderFeatureSet.h,v $
//   $Author: janbo $
//   $Revision: 1.2 $
//   $Date: 2004/10/18 16:22:56 $
//
//  Description: A simple scene class.
//
//=============================================================================

#ifndef AC_Y60_SHADER_FEATURE_SET_INCLUDED
#define AC_Y60_SHADER_FEATURE_SET_INCLUDED

#include <dom/Nodes.h>
#include <y60/DataTypes.h>
#include <asl/Exception.h>

#include <iostream>
#include <string>
#include <vector>
#include <map>

namespace y60 {

typedef std::map<std::string, VectorOfString> FeatureMap;

class ShaderFeatureSet {
    public:
        ShaderFeatureSet();
        virtual ~ShaderFeatureSet();
        const VectorOfString * getFeatures(const std::string & theFeatureClass) const;
        bool hasFeature(const std::string & theFeatureClass) const;
        void load(const dom::NodePtr theNode);
        unsigned getFeatureCount() const;
        
    private:
        FeatureMap _myFeatureMap;
};

} // namespace y60



#endif // AC_Y60_SHADER_FEATURE_SET_INCLUDED
