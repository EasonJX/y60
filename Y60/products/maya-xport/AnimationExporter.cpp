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
#include "ExportExceptions.h"
#include "MayaHelpers.h"

#include <y60/SceneBuilder.h>
#include <y60/AnimationBuilder.h>
#include <y60/TransformBuilder.h>
#include <y60/CharacterBuilder.h>
#include <y60/ClipBuilder.h>

#include <asl/numeric_functions.h>
#include <asl/string_functions.h>

#include <maya/MObject.h>
#include <maya/MFnCamera.h>
#include <maya/MFnCharacter.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MStatus.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MFnTransform.h>
#include <maya/MFnClip.h>

#include <iostream>
#include <float.h>

using namespace std;
using namespace y60;
using namespace asl;

#define DB(x) // x

void
doNotConvert(float & theValue) {
}

template <class T>
void
AnimationExporter::exportSampledData(y60::AnimationBuilder & theAnimBuilder,
                                     const std::vector<T> & theValues,
                                     const std::string & theCurveName,
                                     const MFnAnimCurve::InfinityType & thePostInfinity,
                                     const std::string & theNodeId,
                                     const std::string & theAttributeName,
                                     MTime theStartTime,
                                     MTime theDuration) 
{
    START_TIMER(exportAnimation_myAnimationBuilder);
    theAnimBuilder.setName(theCurveName);
    theAnimBuilder.setNodeRef(theNodeId);
    theAnimBuilder.setAttribute(theAttributeName);
    theAnimBuilder.setDuration(float(theDuration.as(MTime::kSeconds)));
    STOP_TIMER(exportAnimation_myAnimationBuilder);

    theAnimBuilder.setBegin(float(theStartTime.as(MTime::kSeconds)));

    START_TIMER(exportAnimation_myAnimationBuilder_setCount);
    // MFnAnimCurve::InfinityType myPostInfinity = theAnimCurve.postInfinityType();
    if (thePostInfinity == MFnAnimCurve::kCycle) {
        theAnimBuilder.setCount(0);
    } else if (thePostInfinity == MFnAnimCurve::kOscillate) {
        theAnimBuilder.setDirection(y60::PONG);
        theAnimBuilder.setCount(0);
    }
    STOP_TIMER(exportAnimation_myAnimationBuilder_setCount);

    START_TIMER(exportAnimation_appendValues);
    theAnimBuilder.setBegin(float(theStartTime.as(MTime::kSeconds)));
    theAnimBuilder.appendValues(theValues);
    STOP_TIMER(exportAnimation_appendValues);
}


template <class T>
void
AnimationExporter::exportAnimation(const MFnDagNode & theDagNode,
                                   const std::string & theParameterName,
                                   const std::string & theNodeId,
                                   const std::string & theAttributeName,
                                   ConvertFunc theConvertFunc)
{
    MAKE_SCOPE_TIMER(AnimationExporter_exportAnimation);
    MObject myNode = getAnimationNode(theDagNode, theParameterName);
    if (!myNode.isNull()) {
        DB(AC_TRACE << "animation node found for " << theParameterName << endl);
        MStatus myStatus;
        MFnAnimCurve myAnimCurve(myNode, &myStatus);
        if (myStatus == MStatus::kFailure) {
            return;
        }
        y60::AnimationBuilder myAnimationBuilder;
        std::string myId = _mySceneBuilder.appendAnimation(myAnimationBuilder);
        bool success = exportCurve<T>(myAnimationBuilder, myAnimCurve, theDagNode,
                       theParameterName, theNodeId, theAttributeName, theConvertFunc);
        if (!success) {
            _mySceneBuilder.removeNodeById(_mySceneBuilder.getNode()->childNode(y60::ANIMATION_LIST_NAME), myId);
        }
    }
}

AnimationExporter::AnimationExporter(y60::SceneBuilder & theSceneBuilder) :
            _mySceneBuilder(theSceneBuilder)
{
    // build my description map
    _myMayaAttribDescriptionMap["translateX"] = ValueDescriptionPtr(new ValueDescription("position.x", &convertToMeter, "float"));
    _myMayaAttribDescriptionMap["translateY"] = ValueDescriptionPtr(new ValueDescription("position.y", &convertToMeter, "float"));
    _myMayaAttribDescriptionMap["translateZ"] = ValueDescriptionPtr(new ValueDescription("position.z", &convertToMeter, "float"));


    _myMayaAttribDescriptionMap["rotateX"] = ValueDescriptionPtr(new ValueDescription("orientation.x", &convertToRad, "float"));
    _myMayaAttribDescriptionMap["rotateY"] = ValueDescriptionPtr(new ValueDescription("orientation.y", &convertToRad, "float"));
    _myMayaAttribDescriptionMap["rotateZ"] = ValueDescriptionPtr(new ValueDescription("orientation.z", &convertToRad, "float"));


    _myMayaAttribDescriptionMap["scaleX"] = ValueDescriptionPtr(new ValueDescription("scale.x", &doNotConvert, "float"));
    _myMayaAttribDescriptionMap["scaleY"] = ValueDescriptionPtr(new ValueDescription("scale.y", &doNotConvert, "float"));
    _myMayaAttribDescriptionMap["scaleZ"] = ValueDescriptionPtr(new ValueDescription("scale.z", &doNotConvert, "float"));

    _myMayaAttribDescriptionMap["visibility"] = ValueDescriptionPtr(new ValueDescription("visible", &doNotConvert, "bool"));
}

AnimationExporter::~AnimationExporter() {
}

asl::Vector3f
AnimationExporter::getInitialOrientation(const MFnDagNode & theDagNode) {
    MStatus myStatus;
    MFnTransform myTransform(theDagNode.object(), &myStatus);
    if (!myStatus) {
        throw ExportException(string("Could not cast a DagNode to MFnTransform: ")+
                myStatus.errorString().asChar(), PLUS_FILE_LINE);
    }
    MTransformationMatrix::RotationOrder myOrder = MTransformationMatrix::kXYZ;
    double myRotation[3];
    myStatus = myTransform.getRotation(myRotation, myOrder);
    if (!myStatus) {
        throw ExportException(string("Could not get rotation from MFnTransform: ")+
                myStatus.errorString().asChar(), PLUS_FILE_LINE);
    }
    
    return asl::Vector3f(float(myRotation[0]), float(myRotation[1]), float(myRotation[2]));    
}

void
AnimationExporter::exportCharacter(const MObject & theNode, std::map<std::string, std::string> & theIdMap) {
    MFnCharacter myCharacterNode(theNode);
    MStatus myStatus;
    CharacterBuilder myCharacterBuilder(myCharacterNode.name().asChar());
    _mySceneBuilder.appendCharacter(myCharacterBuilder);

    // get character clips
    int myNumClips 	= myCharacterNode.getSourceClipCount(&myStatus);
    if (myStatus != MStatus::kSuccess) {
        throw ExportException(std::string("Character does not have any clips."),
                PLUS_FILE_LINE);
    }
    DB(AC_TRACE << "exportCharacter() found clips: " <<  myNumClips << endl;)
	///
	for (unsigned int i = 0; i < myNumClips; i++) {
        RotationMap myRotations;

	    MObject	myClipObject = myCharacterNode.getSourceClip(i, &myStatus);
        if (myStatus != MStatus::kSuccess) {
            throw ExportException(std::string("Sorry, can't retrieve clip from character."),
                    PLUS_FILE_LINE);
        }
        MFnClip myClip(myClipObject);
        ClipBuilder myClipBuilder(myClip.name().asChar());
        myCharacterBuilder.appendClip(myClipBuilder);
        MObjectArray myCurves;
        MPlugArray mySrcPlugArray;
        myStatus = myClip.getMemberAnimCurves(myCurves, mySrcPlugArray);
        if (myStatus != MStatus::kSuccess) {
            throw ExportException(std::string("Sorry, can't retrieve curves and plugs from clip."),
                    PLUS_FILE_LINE);
        }
        unsigned int myNumCurves = myCurves.length();
        unsigned int myNumPlugs = mySrcPlugArray.length();
        if (myNumCurves != myNumPlugs) {
            throw ExportException(std::string("Sorry, number of curves: ") + as_string(myNumCurves)  +
                                                " and plugs: " +  as_string(myNumPlugs) + " must match.",
                                                PLUS_FILE_LINE);
        }
        DB(AC_TRACE << "number of curves; " <<  myNumCurves << endl;)
        DB(AC_TRACE << "number of plugs; " <<  myNumPlugs << endl;)
        for (unsigned myCurveIndex = 0; myCurveIndex < myNumCurves; ++myCurveIndex) {
            const MPlug &myPlug = mySrcPlugArray[myCurveIndex];
            // we do not support subcharacters yet
		    if (myPlug.node().hasFn (MFn::kCharacter)) {
                throw ExportException(std::string("Sorry, subcharacters are not supported yet."),
                        PLUS_FILE_LINE);
            }
            string myAttributeName(myPlug.partialName(false,false,false,false,false,true).asChar());
            MObject myConnectedTo = myPlug.node();
            MDagPath myPath;
            MDagPath::getAPathTo(myConnectedTo, myPath);
            const string myPathName = myPath.fullPathName().asChar();

            std::map<std::string, std::string>::iterator myMapIt = theIdMap.find(myPathName);
            if (myMapIt == theIdMap.end()) {
                throw ExportException(std::string("Can not find a body with path '")+myPathName+"'",
                        PLUS_FILE_LINE);
            }
            std::string myId = myMapIt->second;

            AnimationBuilder myAnimationBuilder;

            MObject myAttribute = myPlug.attribute();
            string myTypeStr = myAttribute.apiTypeStr();
            MayaAttribDescriptionMap::iterator myMapIter =
                            _myMayaAttribDescriptionMap.find(myAttributeName);
            if (myMapIter == _myMayaAttribDescriptionMap.end()) {
                throw ExportException(
                    std::string("Can not find maya attribute description: ")+myAttributeName,
                    PLUS_FILE_LINE);
            }
            bool success = false;
            if (myMapIter->second->_myTypeName == "float") {
                string::size_type myPos = myMapIter->second->_myY60AttributeName.find("orientation");
                if (myPos != string::npos) {
                    if (myRotations.find(myId) == myRotations.end()) {
                        MFnDagNode myDagNode(myConnectedTo);
                        DB(AC_TRACE << "initial orientation: " << getInitialOrientation(myDagNode) << endl);
                        myRotations.insert(pair<string, RotationContainer>(myId, RotationContainer(getInitialOrientation(myDagNode))));
                    }

                    RotationContainer & myRotationContainer = myRotations[myId];
                    string::size_type myPosX = myMapIter->second->_myY60AttributeName.find("orientation.x");
                    if (myPosX != string::npos) {
                        myRotationContainer.myRotationX = MFnAnimCurvePtr(new MFnAnimCurve(myCurves[myCurveIndex]));
                    } else {
                        string::size_type myPosY = myMapIter->second->_myY60AttributeName.find("orientation.y");
                        if (myPosY != string::npos) {
                            myRotationContainer.myRotationY = MFnAnimCurvePtr(new MFnAnimCurve(myCurves[myCurveIndex]));
                        } else {
                            string::size_type myPosZ = myMapIter->second->_myY60AttributeName.find("orientation.z");
                            if (myPosZ != string::npos) {
                                myRotationContainer.myRotationZ = MFnAnimCurvePtr(new MFnAnimCurve(myCurves[myCurveIndex]));
                            } else {
                                throw ExportException(
                                    std::string("Could not find rotation axis: ")+myAttributeName,
                                    PLUS_FILE_LINE);
                            }
                        }
                    }
                                        
                    success = true;
                } else {
                    myClipBuilder.appendAnimation(myAnimationBuilder);                
                    myAnimationBuilder.setEnable(false);
                    success = exportCurve<float>(myAnimationBuilder,
                                                myCurves[myCurveIndex],
                                                theNode,
                                                myAttributeName, myId,
                                                myMapIter->second->_myY60AttributeName,
                                                myMapIter->second->_myConvertFunc);
                }
            } else if (myMapIter->second->_myTypeName == "bool") {
                    myClipBuilder.appendAnimation(myAnimationBuilder);                
                    myAnimationBuilder.setEnable(false);
                    success = exportCurve<AcBool>(myAnimationBuilder,
                                                myCurves[myCurveIndex],
                                                theNode,
                                                myAttributeName, myId,
                                                myMapIter->second->_myY60AttributeName,
                                                myMapIter->second->_myConvertFunc);
            } else {
                throw ExportException(std::string("Sorry, maya type not yet supported : ") + myTypeStr
                                      + " for attribute: " + myAttributeName,
                        PLUS_FILE_LINE);
            }
            if (!success) {
                myClipBuilder.removeNodeById(
                        myClipBuilder.getNode()->childNode(ANIMATION_LIST_NAME), myId);
            }

        } //next curve
        
        // export all rotation animations per node        
        RotationMap::iterator myIter = myRotations.begin();
        for (;myIter != myRotations.end(); ++myIter) {
            AnimationBuilder myAnimationBuilder;
            myClipBuilder.appendAnimation(myAnimationBuilder);
            std::string myId = myIter->first;

            std::string myAnimCurveName;
            MTime myStartTime(0.0);
            MTime myEndTime(0.0);
            bool foundCurve = false;
            MFnAnimCurve::InfinityType myPostInfinity;
            
            if (myIter->second.myRotationX) {
                foundCurve = true;
                myAnimCurveName = string(myIter->second.myRotationX->name().asChar());
                myStartTime = myIter->second.myRotationX->time(0);
                myEndTime = myIter->second.myRotationX->time(myIter->second.myRotationX->numKeys() - 1);
                myPostInfinity = myIter->second.myRotationX->postInfinityType();
            }
            if (myIter->second.myRotationY) {
                if (foundCurve) {
                    myStartTime = minimum(myIter->second.myRotationY->time(0), myStartTime);
                    myEndTime = maximum(myIter->second.myRotationY->time(myIter->second.myRotationY->numKeys() -1 ), myEndTime);
                } else {
                    myStartTime  = myIter->second.myRotationY->time(0);
                    myEndTime = myIter->second.myRotationY->time(myIter->second.myRotationY->numKeys() - 1);
                    myAnimCurveName = string(myIter->second.myRotationY->name().asChar());
                    myPostInfinity = myIter->second.myRotationY->postInfinityType();
                    foundCurve = true;
                }
            }
            if (myIter->second.myRotationZ) {
                if (foundCurve) {
                    myStartTime = minimum(myIter->second.myRotationZ->time(0), myStartTime);
                    myEndTime = maximum(myIter->second.myRotationZ->time(myIter->second.myRotationZ->numKeys() -1 ), myEndTime);
                } else {
                    myStartTime  = myIter->second.myRotationZ->time(0);
                    myEndTime = myIter->second.myRotationZ->time(myIter->second.myRotationZ->numKeys() - 1);
                    myAnimCurveName = string(myIter->second.myRotationZ->name().asChar());
                    myPostInfinity = myIter->second.myRotationZ->postInfinityType();
                    foundCurve = true;
                }
            }
            
            exportQuaternionAnimation(myAnimationBuilder, myIter->second.myRotationX, myIter->second.myRotationY, myIter->second.myRotationZ,
                                      myStartTime, myEndTime, myIter->second.myInitialEuler,
                                      myId, "orientation", myPostInfinity, myAnimCurveName);
        }
    } //next clip
}

void
AnimationExporter::exportGlobal(const MFnDagNode & theDagNode, const std::string & theNodeId) {
    DB(AC_TRACE << "exportAnimation() examining node: " << theDagNode.name().asChar() << endl;)
    MAKE_SCOPE_TIMER(AnimationExporter_exportCurves);

    exportAnimation<float>(theDagNode, "translateX", theNodeId, "position.x", &convertToMeter);
    exportAnimation<float>(theDagNode, "translateY", theNodeId, "position.y", &convertToMeter);
    exportAnimation<float>(theDagNode, "translateZ", theNodeId, "position.z", &convertToMeter);

    exportRotatingAnimation(theDagNode, theNodeId);

    exportAnimation<float>(theDagNode, "scaleX", theNodeId, "scale.x", &doNotConvert);
    exportAnimation<float>(theDagNode, "scaleY", theNodeId, "scale.y", &doNotConvert);
    exportAnimation<float>(theDagNode, "scaleZ", theNodeId, "scale.z", &doNotConvert);

    exportAnimation<AcBool>(theDagNode, "visibility", theNodeId, "visible", &doNotConvert);

    if (theDagNode.object().apiType() == MFn::kCamera) {
        exportAnimation<float>(theDagNode, "focalLength", theNodeId, "hfov", &doNotConvert);
    }
}

void
AnimationExporter::exportQuaternionAnimation(y60::AnimationBuilder & theAnimBuilder,
                                             MFnAnimCurvePtr theAnimCurveX,
                                             MFnAnimCurvePtr theAnimCurveY,
                                             MFnAnimCurvePtr theAnimCurveZ,
                                             MTime & theStartTime,
                                             MTime & theEndTime,
                                             const asl::Vector3f & theInitialEuler,
                                             const std::string & theNodeId,
                                             const std::string & theAttributeName,
                                             const MFnAnimCurve::InfinityType & thePostInfinity,
                                             const std::string & theAnimCurveName)
{
    DB(AC_TRACE << "Exporting animation rotation: " << theStartTime << "-" << theEndTime << std::endl);

    MTime myDuration   = theEndTime - theStartTime;

    MTime mySampleTime = theStartTime;
    mySampleTime.setUnit(MTime::uiUnit());
    
    std::vector<asl::Quaternionf> myValues;
    unsigned mySampleCount = static_cast<unsigned>((theEndTime - theStartTime).value());
    myValues.reserve(mySampleCount);

    MAKE_SCOPE_TIMER(exportAnimation_loop);

    for (; mySampleTime <= theEndTime; mySampleTime++) {
        START_TIMER(exportAnimation_loop_evaluate);
        asl::Vector3f myCurrentEuler(theInitialEuler);

        if (theAnimCurveX) {
            myCurrentEuler[0] = float(theAnimCurveX->evaluate(mySampleTime));
            convertToRad(myCurrentEuler[0]);
        }
        if (theAnimCurveY) {
            myCurrentEuler[1] = float(theAnimCurveY->evaluate(mySampleTime));
            convertToRad(myCurrentEuler[1]);
        }
        if (theAnimCurveZ) {
            myCurrentEuler[2] = float(theAnimCurveZ->evaluate(mySampleTime));
            convertToRad(myCurrentEuler[2]);
        }

        STOP_TIMER(exportAnimation_loop_evaluate);
        MAKE_SCOPE_TIMER(exportAnimation_loop_push_back);
        myValues.push_back(Quaternionf::createFromEuler(myCurrentEuler));
        DB(AC_TRACE << "Exporting " << theAttributeName << ": " << "Euler=" << myCurrentEuler << "| Quat=" << Quaternionf::createFromEuler(myCurrentEuler) << std::endl);
    }
    
    exportSampledData(theAnimBuilder, myValues, theAnimCurveName, thePostInfinity,
            theNodeId, theAttributeName, theStartTime, myDuration);
            
}



bool
AnimationExporter::exportCurve(y60::AnimationBuilder & theAnimBuilder,
                               const MObject & theAnimNodeX,
                               const MObject & theAnimNodeY,
                               const MObject & theAnimNodeZ,
                               const asl::Vector3f & theInitialEuler,
                               const std::string & theNodeId,
                               const std::string & theAttributeName)
{
    MFnAnimCurvePtr myAnimCurveX(0);
    MFnAnimCurvePtr myAnimCurveY(0);
    MFnAnimCurvePtr myAnimCurveZ(0);
    MStatus myStatus;

    std::string myAnimCurveName;
    MTime myStartTime(0.0);
    MTime myEndTime(0.0);
    MFnAnimCurve::InfinityType myPostInfinity;
    bool foundCurve = false;

    if (!theAnimNodeX.isNull()) {
        myAnimCurveX = MFnAnimCurvePtr(new MFnAnimCurve(theAnimNodeX, &myStatus));
        myStartTime  = myAnimCurveX->time(0);
        myEndTime = myAnimCurveX->time(myAnimCurveX->numKeys() - 1);
        foundCurve = true;
        myAnimCurveName = string(myAnimCurveX->name().asChar());
        myPostInfinity = myAnimCurveX->postInfinityType();
    }
    if (!theAnimNodeY.isNull()) {
        myAnimCurveY = MFnAnimCurvePtr(new MFnAnimCurve(theAnimNodeY, &myStatus));
        if (foundCurve) {
            myStartTime = minimum(myAnimCurveY->time(0), myStartTime);
            myEndTime = maximum(myAnimCurveY->time(myAnimCurveY->numKeys() -1 ), myEndTime);
        } else {
            myStartTime  = myAnimCurveY->time(0);
            myEndTime = myAnimCurveY->time(myAnimCurveY->numKeys() - 1);
            myAnimCurveName = string(myAnimCurveY->name().asChar());
            myPostInfinity = myAnimCurveY->postInfinityType();
            foundCurve = true;
        }
    }
    if (!theAnimNodeZ.isNull()) {
        myAnimCurveZ = MFnAnimCurvePtr(new MFnAnimCurve(theAnimNodeZ, &myStatus));
        if (foundCurve) {
            myStartTime = minimum(myAnimCurveZ->time(0), myStartTime);
            myEndTime = maximum(myAnimCurveZ->time(myAnimCurveZ->numKeys() -1 ), myEndTime);
        } else {
            myStartTime  = myAnimCurveZ->time(0);
            myEndTime = myAnimCurveZ->time(myAnimCurveZ->numKeys() - 1);
            myAnimCurveName = string(myAnimCurveZ->name().asChar());
            myPostInfinity = myAnimCurveZ->postInfinityType();
            foundCurve = true;
        }
    }

    if (!foundCurve || myAnimCurveName == "") {
        return false;
    }

    exportQuaternionAnimation(theAnimBuilder, myAnimCurveX, myAnimCurveY, myAnimCurveZ,
                              myStartTime, myEndTime, theInitialEuler,
                              theNodeId, theAttributeName, myPostInfinity, myAnimCurveName);

    return true;
}

void
AnimationExporter::exportRotatingAnimation(const MFnDagNode & theDagNode, const std::string & theNodeId) {
    MAKE_SCOPE_TIMER(AnimationExporter_exportAnimation);
    MStatus myStatus;
    MObject myXNode = getAnimationNode(theDagNode, "rotateX");
    MObject myYNode = getAnimationNode(theDagNode, "rotateY");
    MObject myZNode = getAnimationNode(theDagNode, "rotateZ");
    if ( ! (myXNode.isNull() && myYNode.isNull() && myZNode.isNull() ) ) {        
        y60::AnimationBuilder myAnimationBuilder;
        std::string myId = _mySceneBuilder.appendAnimation(myAnimationBuilder);

        bool success = exportCurve(myAnimationBuilder, myXNode, myYNode, myZNode,
                                   getInitialOrientation(theDagNode),
                                    theNodeId, "orientation");

        if (!success) {
            _mySceneBuilder.removeNodeById(_mySceneBuilder.getNode()->childNode(ANIMATION_LIST_NAME), myId);
        }
    }
}

MObject
AnimationExporter::getAnimationNode(const MFnDagNode & theDagNode,
                                    const std::string & theParameterName)
{
    MAKE_SCOPE_TIMER(AnimationExporter_getAnimationNode);
    MStatus myStatus;
    MPlug myPlug = theDagNode.findPlug(theParameterName.c_str(), &myStatus);
    if (myStatus == MStatus::kSuccess) {
        MPlugArray myConnectedPlugs;
        myPlug.connectedTo(myConnectedPlugs, true, false, &myStatus);
        if (myStatus == MStatus::kSuccess && myConnectedPlugs.length() == 1 &&
            !myConnectedPlugs[0].node().hasFn (MFn::kCharacter)) {
            return myConnectedPlugs[0].node();
        }
    }
    return MObject();
}

void
AnimationExporter::printAnimationInfo(const MFnAnimCurve & theAnimation) {
    cerr << "Animation type: " << theAnimation.animCurveType() << endl;
    cerr << "Number of keys: " << theAnimation.numKeys() << endl;
    cerr << "Time and value for key 0: " << theAnimation.time(0).value() << " | " << theAnimation.value(0) << endl;
    cerr << "Time and value for key 1: " << theAnimation.time(1).value() << " | " << theAnimation.value(1) << endl;
    cerr << "Value at time 250: " << theAnimation.evaluate(MTime(250.0)) << endl;

    MTime myEndTime    = theAnimation.time(theAnimation.numKeys() - 1);
    MTime mySampleTime = theAnimation.time(0);
    mySampleTime.setUnit(MTime::uiUnit());

    cerr << "start: " << mySampleTime.as(MTime::kSeconds) << " end: " << myEndTime.as(MTime::kSeconds) << endl;
    cerr << "AnimValues: " << endl;
    for (; mySampleTime <= myEndTime; mySampleTime++) {
        //cerr << mySampleTime.as(MTime::kSeconds) << ": " << theAnimation.evaluate(mySampleTime) << " | ";
    }
    cerr << endl;
}


template <class T>
bool
AnimationExporter::exportCurve(y60::AnimationBuilder & theAnimBuilder,
                               const MFnAnimCurve & theAnimCurve,
                               const MFnDagNode & theDagNode,
                               const std::string & theParameterName,
                               const std::string & theNodeId,
                               const std::string & theAttributeName,
                               ConvertFunc theConvertFunc)
{
    std::string myAnimCurveName = theAnimCurve.name().asChar();
    // We reject animations with empty names, because we do not know where they come from
    // TODO, find what is happening here!
    if (myAnimCurveName == "") {
        return false;
    }

    DB(AC_TRACE << "Exporting animation curve: " << myAnimCurveName << std::endl);
    
    MTime myEndTime    = theAnimCurve.time(theAnimCurve.numKeys()-1);
    MTime myStartTime  = theAnimCurve.time(0);
    MTime myDuration   = myEndTime - myStartTime;

    DB(AC_TRACE << " start: " << myStartTime << ", end " << myEndTime << std::endl);
        
    MTime mySampleTime = myStartTime;
    // mySampleTime.setUnit(ourSamplingUnit);
    mySampleTime.setUnit(MTime::uiUnit());
    std::vector<T> myValues;
    unsigned mySampleCount = static_cast<unsigned>((myEndTime - myStartTime).value());
    myValues.reserve(mySampleCount);

    // Focal length animation needs special treatment, because it is not
    // trivial to convert focal length to hfov
    if (theParameterName == "focalLength") {
        MAKE_SCOPE_TIMER(exportAnimation_loop_focalLength);
        MFnCamera myCamera(theDagNode.object());
        double myOriginalFocalLength = myCamera.focalLength();
        for (; mySampleTime <= myEndTime; mySampleTime++) {
            double myFocalLength = theAnimCurve.evaluate(mySampleTime);

            // Set the animated focal length
            myCamera.setFocalLength(myFocalLength);

            // Retrieve hfov from camera
            float myValue = float(asl::degFromRad(myCamera.horizontalFieldOfView()));
            myValues.push_back(myValue);
        }
        myCamera.setFocalLength(myOriginalFocalLength);
    } else {
        MAKE_SCOPE_TIMER(exportAnimation_loop);
        for (; mySampleTime <= myEndTime; mySampleTime++) {
            START_TIMER(exportAnimation_loop_evaluate);
            float myValue = float(theAnimCurve.evaluate(mySampleTime));
            DB(AC_TRACE << "====== Sample @ " << mySampleTime << " is " << myValue << std::endl);
            STOP_TIMER(exportAnimation_loop_evaluate);
            START_TIMER(exportAnimation_theConvertFunc);
            theConvertFunc(myValue);
            STOP_TIMER(exportAnimation_theConvertFunc);
            MAKE_SCOPE_TIMER(exportAnimation_loop_push_back);
            myValues.push_back(static_cast<T>(myValue));
        }
    }
    exportSampledData(theAnimBuilder, myValues, theAnimCurve.name().asChar(), 
            theAnimCurve.postInfinityType(), theNodeId, theAttributeName, 
            myStartTime, myDuration);
    return true;
}


