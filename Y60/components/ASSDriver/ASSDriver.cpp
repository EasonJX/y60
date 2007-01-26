//============================================================================
// Copyright (C) 2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#include "ASSDriver.h"
#include "ConnectedComponent.h"

#include <asl/Logger.h>
#include <asl/SerialDeviceFactory.h>
#include <asl/Assure.h>

#include <y60/ImageBuilder.h>
#include <y60/PixelEncoding.h>
#include <y60/AbstractRenderWindow.h>

#include <iostream>

using namespace asl;
using namespace y60;

const unsigned char myMagicToken( 255 ); 

static const char * RAW_RASTER = "ASSRawRaster";
static const char * BINARY_RASTER = "ASSBinaryRaster";

static const char * DriverStateStrings[] = {
    "synchronizing",
    "running",
    ""
};

IMPLEMENT_ENUM( DriverState, DriverStateStrings );


ASSDriver::ASSDriver(DLHandle theDLHandle) :
    PlugInBase(theDLHandle),
    IRendererExtension("ASSDriver"),
    _myGridSize(0,0),
    _mySyncLostCounter(0),
    _myRawRaster(dom::ValuePtr(0)),
    _myDenoisedRaster(dom::ValuePtr(0)),
    _myScene(0),
    _myNoiseThreshold( 15 ),
    _myComponentThreshold( 30 ),
    _myPower(1)
{
    _mySerialPort = getSerialDevice(0);
    _mySerialPort->open( 57600, 8, SerialDevice::NO_PARITY, 1, false);
    setState(SYNCHRONIZING);
}

ASSDriver::~ASSDriver() {
    AC_PRINT << "ASSDriver stats:";
    AC_PRINT << "    sync errors: " << _mySyncLostCounter;
}

bool
ASSDriver::onSceneLoaded(jslib::AbstractRenderWindow * theWindow) {
    _myScene = theWindow->getCurrentScene();
    return true;
}

void
ASSDriver::onFrame(jslib::AbstractRenderWindow * theWindow, double theTime) {
}

void
ASSDriver::synchronize() {
    static int myLineStart = -1;
    static int myLineEnd   = -1;
    static int myMaxLine = 0;
    static bool myMagicTokenFlag = false;
    // first look for the beginning of a line
    if (myLineStart < 0) {
        for (unsigned i = 0; i < _myBuffer.size(); ++i) {
            if (myMagicTokenFlag) {
                myMaxLine = _myBuffer[i];
                myMagicTokenFlag = false;
                myLineStart = i;
                break;
            } else if (_myBuffer[i] == myMagicToken) {
                myMagicTokenFlag = true;
            }
        }
    }
    // then find the end of that line and determine the number of columns
    if (myLineStart >= 0 && myLineEnd < 0) {
        for (unsigned i = myLineStart + 1; i < _myBuffer.size(); ++i) {
            if (_myBuffer[i] == myMagicToken) {
                myLineEnd = i;
                _myGridSize[0] = myLineEnd - myLineStart - 1;
                break;
            }
        }
    }

    // now scan for the maximum line number to determine the number of rows
    if (_myGridSize[0] > 0 && _myGridSize[1] == 0) {
        ASSURE( _myBuffer[ myLineEnd ] == myMagicToken );
        for (unsigned i = myLineEnd; i < _myBuffer.size(); i += _myGridSize[0] + 2) {
            ASSURE( _myBuffer[i] == myMagicToken );
            if (_myBuffer[i + 1] == 1 && myMaxLine > 0) {
                _myGridSize[1] = myMaxLine;
                AC_PRINT << "Grid size: " << _myGridSize;
                _myBuffer.erase( _myBuffer.begin(), _myBuffer.begin() + i);
                ASSURE(_myBuffer.front() == myMagicToken);
                // reset the statics so we could resynchronize ... at least in theory
                myLineStart = -1;
                myLineEnd = -1;
                myMaxLine = 0;
                myMagicTokenFlag = false;
                allocateGridBuffers();
                setState( RUNNING );
                return;
            } else {
                myMaxLine = _myBuffer[i + 1];
            }
        }
    }
}

void
ASSDriver::allocateGridBuffers() {
    _myRawRaster = allocateRaster(RAW_RASTER);
    _myDenoisedRaster = allocateRaster(BINARY_RASTER);
}

RasterHandle
ASSDriver::allocateRaster(const std::string & theName) {
    if (_myScene) {
        dom::NodePtr myImage = _myScene->getSceneDom()->getElementById(theName);
        if (myImage) {
            myImage->getFacade<y60::Image>()->createRaster( 
                    _myGridSize[0], _myGridSize[1], 1, y60::GRAY);
            return RasterHandle( myImage->childNode(0)->childNode(0)->nodeValueWrapperPtr());
        } else {
            ImageBuilder myImageBuilder(theName, false);
            _myScene->getSceneBuilder()->appendImage(myImageBuilder);
            y60::ImagePtr myImage = myImageBuilder.getNode()->getFacade<y60::Image>();
            myImage->set<y60::IdTag>( theName );
            myImage->createRaster( _myGridSize[0], _myGridSize[1], 1, y60::GRAY);
            return RasterHandle( myImage->getNode().childNode(0)->childNode(0)->nodeValueWrapperPtr());
        }
    } else {
        RasterHandle myHandle(createRasterValue(y60::GRAY, _myGridSize[0], _myGridSize[1]));
        return myHandle;
                
    }
}

void
ASSDriver::readSensorValues() {
    //AC_PRINT << "====";
    //AC_PRINT << "pre buf size: " << _myBuffer.size();
    bool myNewDataFlag = false;
    while ( _myGridSize[0] + 2 < _myBuffer.size() ) {
        if ( _myBuffer[0] != myMagicToken ) {
            AC_PRINT << "Sync error.";
            _mySyncLostCounter++;
            setState( SYNCHRONIZING );
            return;
        }
        int myRowIdx = _myBuffer[1] - 1;
        //AC_PRINT << "Got row: " << myRowIdx;

        unsigned char * myRowPtr = _myRawRaster.raster->pixels().begin() +
                    myRowIdx * _myGridSize[0];
        std::copy(_myBuffer.begin() + 2, _myBuffer.begin() + 2 + _myGridSize[0], myRowPtr);
        //*(_myRawRaster.raster->pixels().begin() + (_myGridSize[1] / 2) * _myGridSize[0] + _myGridSize[0] / 2) = 254;

        _myBuffer.erase( _myBuffer.begin(), _myBuffer.begin() + _myGridSize[0] + 2);
        if ( myRowIdx == _myGridSize[1] - 1) {
            asl::Time myTime;
            double myDeltaT = myTime - _myLastFrameTime;
            //AC_PRINT << "Got Frame (dt = " << myDeltaT << ")";
            _myLastFrameTime = myTime;

            processSensorValues();
        }
        myNewDataFlag = true;
    }

    if (myNewDataFlag && _myScene) {
        _myScene->getSceneDom()->getElementById(RAW_RASTER)->getFacade<y60::Image>()->triggerUpload();
        
    }
    //AC_PRINT << "post buf size: " << _myBuffer.size();
}

template <class PixelT>
struct ThresholdWithPow {
    ThresholdWithPow(const PixelT & theThreshold, int thePower) :
        _myThreshold( theThreshold ),
        _myPower( thePower) {}
    PixelT operator () (const PixelT & a) {
        if (a.get() < _myThreshold.get()) {
            return PixelT(0);
        } else {
            return PixelT( ipow(int(a.get()), _myPower) /  ipow(255, _myPower - 1));
        }
    }

    PixelT _myThreshold;
    int _myPower;
};

template <class RASTER>
struct AnalyseMoment {
    void operator() (const RASTER & theRaster) {
        analyseMoments( theRaster, result );
    }
    MomentResults result;
};

void
ASSDriver::createThresholdedRaster(RasterHandle & theInput,
                                   RasterHandle & theOutput,
                                   const unsigned char theThreshold)
{
    const y60::RasterOfGRAY * myInput = dom::dynamic_cast_Value<y60::RasterOfGRAY>( & * (theInput.value) );
    y60::RasterOfGRAY * myOutput = dom::dynamic_cast_and_openWriteableValue<y60::RasterOfGRAY>(&* (theOutput.value) );
    std::transform( myInput->begin(), myInput->end(),
            myOutput->begin(),
            ThresholdWithPow<asl::gray<unsigned char> >( theThreshold, _myPower ));
    dom::dynamic_cast_and_closeWriteableValue<y60::RasterOfGRAY>(&* (theOutput.value) );
}

void
ASSDriver::processSensorValues() {
    createThresholdedRaster( _myRawRaster, _myDenoisedRaster, _myNoiseThreshold );

    BlobListPtr myROIs = connectedComponents( _myDenoisedRaster.raster, _myComponentThreshold);

    _myPositions.clear();
    _myRegions.clear();

#if 1
    typedef asl::subraster<gray<unsigned char> > SubRaster;
    for (BlobList::iterator it = myROIs->begin(); it != myROIs->end(); ++it) {
        Box2i myBox = (*it)->bbox();
        myBox[Box2i::MAX] += Vector2i(1, 1);
        AnalyseMoment<SubRaster> myMomentAnalysis;
        _myDenoisedRaster.raster->apply( myBox[Box2i::MIN][0], myBox[Box2i::MIN][1],
                                    myBox[Box2i::MAX][0], myBox[Box2i::MAX][1], myMomentAnalysis);

        Vector2f myPosition = Vector2f( myBox[Box2i::MIN][0], myBox[Box2i::MIN][1]) + myMomentAnalysis.result.center;

        //AC_PRINT << "center: " << myPosition;
        _myPositions.push_back( myPosition );
        _myRegions.push_back( Box2f( Vector2f( myBox[Box2i::MIN][0], myBox[Box2i::MIN][1]),
                                     Vector2f( myBox[Box2i::MAX][0], myBox[Box2i::MAX][1])));
    }

#else // perform moment analysis on whole raster
    if ( ! myROIs->empty()) {
        AnalyseMoment<y60::RasterOfGRAY> myMomentAnalysis;
        _myDenoisedRaster.raster->apply(myMomentAnalysis);

        Vector2f myPosition = myMomentAnalysis.result.center;
        //AC_PRINT << "center: " << myPosition;
        _myPositions.push_back( myPosition );
        _myRegions.push_back( Box2f( Vector2f( 0.0, 0.0), Vector2f( _myGridSize[0], _myGridSize[1])));
    }
#endif
}

void
ASSDriver::setState( DriverState theState ) {
    _myState = theState;
    switch ( _myState ) {
        case SYNCHRONIZING:
            _myGridSize = Vector2i(0, 0);
            _myBuffer.clear();
            break;
        default:
            break;
    }
    AC_PRINT << theState << " ...";
}

void
ASSDriver::onPostRender(jslib::AbstractRenderWindow * theWindow) {
}

void
ASSDriver::onGetProperty(const std::string & thePropertyName,
        PropertyValue & theReturnValue) const
{
    if (thePropertyName == "gridSize") {
        theReturnValue.set( _myGridSize );
        return;
    }
    if (thePropertyName == "maxOccuringValue") {
        theReturnValue.set( myMagicToken );
        return;
    }
    if (thePropertyName == "componentThreshold") {
        theReturnValue.set( _myComponentThreshold );
        return;
    }
    if (thePropertyName == "noiseThreshold") {
        theReturnValue.set( _myNoiseThreshold );
        return;
    }
    if (thePropertyName == "positions") {
        theReturnValue.set( _myPositions );
        return;
    }
    if (thePropertyName == "regions") {
        theReturnValue.set( _myRegions );
        return;
    }
    if (thePropertyName == "gainPower") {
        theReturnValue.set( _myPower );
        return;
    }
    AC_ERROR << "Unknown property '" << thePropertyName << "'";
}

void
ASSDriver::onSetProperty(const std::string & thePropertyName,
        const PropertyValue & thePropertyValue)
{
    if (thePropertyName == "gridSize") {
        AC_ERROR << "gridSize property is read only";
        return;
    }
    if (thePropertyName == "maxOccuringValue") {
        AC_ERROR << "maxOccuringValue property is read only";
        return;
    }
    if (thePropertyName == "componentThreshold") {
        _myComponentThreshold = thePropertyValue.get<unsigned char>();
        return;
    }
    if (thePropertyName == "noiseThreshold") {
        _myNoiseThreshold = thePropertyValue.get<unsigned char>();
        return;
    }
    if (thePropertyName == "gainPower") {
        _myPower = thePropertyValue.get<int>();
        return;
    }
    if (thePropertyName == "positions") {
        AC_ERROR << "positions property is read only";
        return;
    }
    if (thePropertyName == "regions") {
        AC_ERROR << "regions property is read only";
        return;
    }
    AC_ERROR << "Unknown property '" << thePropertyName << "'";
}

void
ASSDriver::onUpdateSettings(dom::NodePtr theSettings) {
}

y60::EventPtrList 
ASSDriver::poll() {
    std::vector<unsigned char> myBuffer;
    unsigned myByteCount = _mySerialPort->peek();
    myBuffer.resize( myByteCount );
    _mySerialPort->read( reinterpret_cast<char*>(& ( * myBuffer.begin())), myByteCount );

    _myBuffer.insert( _myBuffer.end(), myBuffer.begin(), myBuffer.end() );

    switch (_myState) {
        case SYNCHRONIZING:
            synchronize();
            break;
        case RUNNING:
            readSensorValues();
            break;
    }
    return y60::EventPtrList();
}

extern "C"
EXPORT asl::PlugInBase* ASSDriver_instantiatePlugIn(asl::DLHandle myDLHandle) {
    return new ASSDriver(myDLHandle);
}



