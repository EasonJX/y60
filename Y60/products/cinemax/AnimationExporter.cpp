//=============================================================================
// Copyright (C) 2000-2002, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: AnimationExporter.cpp,v $
//   $Author: pavel $
//   $Revision: 1.26 $
//   $Date: 2005/04/24 00:41:20 $
//
//  Description: This class implements a polygon exporter plugin for maya.
//
//=============================================================================

#include "AnimationExporter.h"
#include "SceneExporter.h"
#include "CinemaHelpers.h"

#include <y60/SceneBuilder.h>
#include <y60/AnimationBuilder.h>
#include <y60/TransformBuilder.h>
#include <y60/CharacterBuilder.h>
#include <y60/ClipBuilder.h>

#include <asl/numeric_functions.h>
#include <asl/string_functions.h>

#include <iostream>
#include <float.h>

using namespace std;
using namespace y60;
using namespace asl;

#define DB(x) // x


static const char * C4D_POSITION_X = "Position . X";
static const char * C4D_POSITION_Y = "Position . Y";
static const char * C4D_POSITION_Z = "Position . Z";

static const char * C4D_SCALE_X = "Scale . X";
static const char * C4D_SCALE_Y = "Scale . Y";
static const char * C4D_SCALE_Z = "Scale . Z";

static const char * C4D_ROTATION_X = "Winkel . P";
static const char * C4D_ROTATION_Y = "Winkel . H";
static const char * C4D_ROTATION_Z = "Winkel . B";


AnimationExporter::AnimationExporter(y60::SceneBuilder & theSceneBuilder, BaseDocument * theDocument) :
            _mySceneBuilder(theSceneBuilder), _myDocument(theDocument)
{
    _myAttributeMap[C4D_POSITION_X] = "position.x";
    _myAttributeMap[C4D_POSITION_Y] = "position.y";
    _myAttributeMap[C4D_POSITION_Z] = "position.z";

    _myAttributeMap[C4D_SCALE_X] = "scale.x";
    _myAttributeMap[C4D_SCALE_Y] = "scale.y";
    _myAttributeMap[C4D_SCALE_Z] = "scale.z";


}

AnimationExporter::~AnimationExporter() {
}


void
AnimationExporter::exportCharacter(BaseObject * theNode, const std::string & theNodeId) {
    CTrack* myFirstTrack = theNode->GetFirstCTrack();
    if (myFirstTrack) {
        std::string myName = getString(theNode->GetName());
        CharacterBuilder myCharacterBuilder(myName);
        _mySceneBuilder.appendCharacter(myCharacterBuilder);

        ClipBuilderPtr myClipBuilder(new ClipBuilder(myName));
        myCharacterBuilder.appendClip(*myClipBuilder);
        exportGlobal(theNode, theNodeId, myClipBuilder);
    }
}

void
AnimationExporter::exportGlobal(BaseObject * theNode, const std::string & theNodeId, 
                                ClipBuilderPtr theClipBuilder) {
    std::string myName = getString(theNode->GetName());

    DB(AC_TRACE << "exportAnimation() examining node: " << myName.asChar() << endl;)

    CTrack*          myTrack;
    for (myTrack = theNode->GetFirstCTrack(); myTrack; myTrack = myTrack->GetNext()) {
        WriteTrack(myTrack, theNodeId, theClipBuilder);
    }


    // export rotation if any property is animated
    if (_myRotationMap.size() > 0) {
        exportRotationAnimation(theNodeId, theClipBuilder);
    }
}

bool 
AnimationExporter::WriteTrack(CTrack* theTrack, const std::string & theNodeId, 
                                ClipBuilderPtr theClipBuilder)
{

    std::string myAttributeC4DName = getString(theTrack->GetName());
    if (_myAttributeMap.find(myAttributeC4DName) != _myAttributeMap.end()){
        std::string myAttributeName = _myAttributeMap.find(myAttributeC4DName)->second;
        y60::AnimationBuilder myAnimationBuilder;
        std::string myId = _mySceneBuilder.appendAnimation(myAnimationBuilder);
        myAnimationBuilder.setAttribute(myAttributeName);
        myAnimationBuilder.setName(myAttributeName);

        myAnimationBuilder.setNodeRef(theNodeId);

        std::vector<float> myValues;
        bool myInverseFlag = false;
        if (myAttributeC4DName == C4D_POSITION_Z ) {
            myInverseFlag = true;
        }
        WriteCurve(theTrack->GetCurve(), myValues, myInverseFlag);
        myAnimationBuilder.appendValues(myValues);
        float myFps = _myDocument->GetFps();
        myAnimationBuilder.setDuration(myValues.size() / myFps);
        if (theClipBuilder) {
            theClipBuilder->appendAnimation(myAnimationBuilder);                
        }
    } else {
        if (myAttributeC4DName == C4D_ROTATION_X || myAttributeC4DName == C4D_ROTATION_Y || myAttributeC4DName == C4D_ROTATION_Z) {
            std::vector<float> myValues;
            WriteCurve(theTrack->GetCurve(), myValues, false);
            _myRotationMap[myAttributeC4DName] = myValues;
        }
    }
    return true;
}

bool 
AnimationExporter::exportRotationAnimation(const std::string & theNodeId, 
                                ClipBuilderPtr theClipBuilder) {
    y60::AnimationBuilder myAnimationBuilder;
    std::string myId = _mySceneBuilder.appendAnimation(myAnimationBuilder);
    myAnimationBuilder.setAttribute("orientation");
    myAnimationBuilder.setName("orientation");
    myAnimationBuilder.setNodeRef(theNodeId);
    myAnimationBuilder.setDuration(10.0f);
    
    std::vector<float> & theRotateH = _myRotationMap[C4D_ROTATION_Y];
    std::vector<float> & theRotateP = _myRotationMap[C4D_ROTATION_X];
    std::vector<float> & theRotateB = _myRotationMap[C4D_ROTATION_Z];
    unsigned myMaxValueCount = max(max(theRotateH.size(), theRotateP.size()), theRotateB.size());
    std::vector<asl::Quaternionf> myValues;
    for (unsigned myIndex = 0; myIndex < myMaxValueCount; myIndex++) {
        asl::Vector3f myCurrentEuler(0,0,0);
        if (theRotateH.size()>= myIndex) {
            myCurrentEuler[1] = float(theRotateH[myIndex]);
        }
        if (theRotateP.size()>= myIndex) {
            myCurrentEuler[0] = float(theRotateP[myIndex]);
        }
        if (theRotateB.size()>= myIndex) {
            myCurrentEuler[2] = float(theRotateB[myIndex]);
        }
        myValues.push_back(Quaternionf::createFromEuler(myCurrentEuler));
    }
    myAnimationBuilder.appendValues(myValues);
    float myFps = _myDocument->GetFps();
    myAnimationBuilder.setDuration(myValues.size() / myFps);
    if (theClipBuilder) {
        theClipBuilder->appendAnimation(myAnimationBuilder);                
    }

    return true;
}

bool 
AnimationExporter::WriteCurve(CCurve* theCurve, std::vector<float> & theValues, bool theInverseFlag)
{
     LONG myKeyCount = theCurve->GetKeyCount();
     for (LONG myKeyIndex = 0; myKeyIndex < myKeyCount; myKeyIndex++) {
        CKey * myKey = theCurve->GetKey(myKeyIndex);
        float myValue = myKey->GetValue();
        if (theInverseFlag) {
            myValue *= -1.0f;
        }
        theValues.push_back(myValue);
    }
    return true;
}

