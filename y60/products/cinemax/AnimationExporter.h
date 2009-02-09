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
//   $RCSfile: AnimationExporter.h,v $
//   $Author: pavel $
//   $Revision: 1.20 $
//   $Date: 2005/04/24 00:41:20 $
//
//  Description: This class implements a polygon exporter plugin for maya.
//
//=============================================================================

#ifndef _ac_c4d_AnimationExporter_h_
#define _ac_c4d_AnimationExporter_h_


#include <y60/base/iostream_functions.h>
#include <y60/scene/AnimationBuilder.h>
#include <y60/scene/SceneBuilder.h>
#include <y60/scene/ClipBuilder.h>

#include "c4d_include.h"

#include <string>
#include <vector>
#include <set>
#include <map>

#undef DB
#define DB(x) // x

namespace y60 {
    class SceneBuilder;
}
class SceneExporter;

typedef std::map<std::string, std::string> AttributeMap;
typedef std::map<std::string, std::vector<float> > RotationMap;
class AnimationExporter {
    public:
        AnimationExporter(y60::SceneBuilder & theSceneBuilder, BaseDocument * theDocument);
        virtual ~AnimationExporter();

        void exportCharacter(BaseObject * theNode, const std::string & theNodeId);
        void exportGlobal(BaseObject * theNode, const std::string & theNodeId,
                          y60::ClipBuilderPtr theClipBuilder = y60::ClipBuilderPtr(0));

    private:
        bool WriteTrack(CTrack* theTrack, const std::string & theNodeId, 
                        y60::ClipBuilderPtr theClipBuilder);
        bool WriteCurve(CCurve* theCurve, std::vector<float> & theValues, bool theInverseFlag);
        bool exportRotationAnimation(const std::string & theNodeId,
                                     y60::ClipBuilderPtr theClipBuilder);
        y60::SceneBuilder & _mySceneBuilder;
        BaseDocument     *  _myDocument;
        AttributeMap        _myAttributeMap;
        RotationMap         _myRotationMap;
};


#endif