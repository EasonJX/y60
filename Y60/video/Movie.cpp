//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "Movie.h"
#include "MovieDecoderBase.h"
#include "M60Decoder.h"

#include <y60/PixelEncoding.h>
#include <y60/DecoderManager.h>
#include <y60/IScene.h>
#include <asl/Logger.h>
#include <asl/PackageManager.h>
#include <asl/PlugInManager.h>
#include <asl/Assure.h>
#include <asl/Dashboard.h>
#include <string.h>

using namespace dom;
using namespace std;
using namespace asl;

#define DB(x) x
#define DB2(x) // x


namespace y60 {

    Movie::Movie(dom::Node & theNode):
        Image(theNode),
        FrameCountTag::Plug(theNode),
        CurrentFrameTag::Plug(theNode),
        FrameRateTag::Plug(theNode),
        PlaySpeedTag::Plug(theNode),
        PlayModeTag::Plug(theNode),
        VolumeTag::Plug(theNode),
        LoopCountTag::Plug(theNode),
        CacheSizeTag::Plug(theNode),
        AVDelayTag::Plug(theNode),
        AudioTag::Plug(theNode),
        DecoderHintTag::Plug(theNode),
        dom::DynamicAttributePlug<MovieTimeTag, Movie>(this, &Movie::getMovieTime),
        dom::DynamicAttributePlug<DecoderTag, Movie>(this, &Movie::getDecoderName),
        _myDecoder(0),
        _myLastDecodedFrame(UINT_MAX),
        _myLastCurrentTime(-1.0),
        _myCurrentLoopCount(0),
        _myPlayMode(PLAY_MODE_STOP)
    {
        AC_DEBUG  << "Movie::Movie " << (void*)this;
        if (getNode()) {
            setup();
        }
    }

    void Movie::setup() {

        //check if we have an inline movie, if so open it using FFMpeg
        dom::NodePtr myBinaryElement = getNode().firstChild();
        if (myBinaryElement && !getRasterPtr()) {
            AC_DEBUG << "found an inline movie for " << get<ImageSourceTag>();

            //DK hold this as a member, otherwise our ReadableStream goes out of scope :-(
            _myStreamData = myBinaryElement->firstChild()->nodeValueWrapperPtr();

            asl::ReadableStream * myStream
                = const_cast<asl::ReadableBlock*>(&_myStreamData->accessReadableBlock());

            loadStream( asl::Ptr<asl::ReadableStream>(myStream), get<ImageSourceTag>());
        }
    }

    Movie::~Movie() {
        AC_DEBUG  << "Movie::~Movie " << (void*)this;
    }

    void Movie::stop() {
        AC_DEBUG  << "Movie::stop";
        if (_myDecoder) {
            _myDecoder->stopMovie();
        }
        set<PlayModeTag>(asl::getStringFromEnum(PLAY_MODE_STOP, MoviePlayModeStrings));
        set<CurrentFrameTag>(0);
        _myLastDecodedFrame = UINT_MAX;
        _myCurrentLoopCount = 0;
        dom::ResizeableRasterPtr myRaster = getRasterPtr();
        if (myRaster) {
            memset(myRaster->pixels().begin(), 0, myRaster->pixels().size());
        }
    }

    void
    Movie::restart(double theCurrentTime) {
        AC_DEBUG  << "Movie::restart";
        _myDecoder->stopMovie();
        _myDecoder->startMovie(0);
        double myMovieTime = _myDecoder->getMovieTime(theCurrentTime);
        decodeFrame(myMovieTime, 0);
    }

    void
    Movie::setPlayMode(MoviePlayMode thePlayMode) {
        AC_DEBUG << "Movie::setPlayMode " << asl::getStringFromEnum(thePlayMode, MoviePlayModeStrings);
        // process changes
        switch (thePlayMode) {
            case PLAY_MODE_STOP:
                stop();
                break;
            case PLAY_MODE_PLAY:
                if (_myPlayMode == PLAY_MODE_PAUSE) {
                    _myDecoder->resumeMovie(getTimeFromFrame(get<CurrentFrameTag>()));
                } else {
                    _myDecoder->startMovie(getTimeFromFrame(get<CurrentFrameTag>()));
                }
                break;
            case PLAY_MODE_PAUSE:
                if (_myPlayMode == PLAY_MODE_STOP) {
                    _myDecoder->startMovie(0);
                }
                _myDecoder->pauseMovie();
                break;
        }

        // Synchronize internal and external representation
        _myPlayMode = thePlayMode;
        set<PlayModeTag>(asl::getStringFromEnum(thePlayMode, MoviePlayModeStrings));
    }

    double Movie::decodeFrame(double theTime, unsigned theFrame) {
        DB(AC_DEBUG << "Movie::decodeFrame time=" << theTime << " frame=" << theFrame);
        double myReturnTime = _myDecoder->readFrame(theTime, theFrame, getRasterPtr());
        
        if (myReturnTime != theTime) {
            _myLastDecodedFrame = getFrameFromTime(myReturnTime);
        } else {
            _myLastDecodedFrame = theFrame; // M60
        }
        set<CurrentFrameTag>(_myLastDecodedFrame);
        return myReturnTime;
    }

    double Movie::getTimeFromFrame(unsigned theFrame) const {
        int FrameCount = get<FrameCountTag>();
        if (FrameCount == -1) {
            return (double)(theFrame) / get<FrameRateTag>();
        } else {
            return (double)(theFrame % get<FrameCountTag>()) / get<FrameRateTag>();
        }
    }
    
    void 
    Movie::ensureMovieFramecount() {
        AC_DEBUG<<"Movie::ensureMovieFramecount "<<get<FrameCountTag>();
        if (get<FrameCountTag>() == -1) {            
            load(AppPackageManager::get().getPtr()->getSearchPath());
            if (get<FrameCountTag>() == -1) {
                MoviePlayMode myPlayMode = MoviePlayMode(asl::getEnumFromString(get<PlayModeTag>(), MoviePlayModeStrings));            
                setPlayMode(PLAY_MODE_PLAY);
                _myDecoder->setDecodeAudioFlag(false);
                readFrame();      
                int myFrame = -1;
                while (!_myDecoder->getEOF()) {
                    myFrame++;
                    double myMovieTime = getTimeFromFrame(myFrame);
                    decodeFrame(myMovieTime, myFrame);                                
                }
                _myDecoder->setEOF(false);  
                restart(0);   
                //_myDecoder->stopMovie();
                _myDecoder->setDecodeAudioFlag(true);
                msleep(500); // give the decoder thread some time to initialize
                setPlayMode(myPlayMode);
            }
        } 
    }

    unsigned Movie::getFrameFromTime(double theTime) const {
        double myFrameHelper = theTime * get<FrameRateTag>();
        int myFrame = int(asl::round(myFrameHelper));
        if (get<FrameCountTag>() == -1) {
            return (unsigned)myFrame;
        } else {
            while (myFrame < 0) {
                myFrame += get<FrameCountTag>();
            }
            return (unsigned)(myFrame % get<FrameCountTag>());
        }
    }

    void
    Movie::readFrame() {
        readFrame(0, true);
    }

    void
    Movie::readFrame(double theCurrentTime, bool theIgnoreCurrentTime) {
        DB(AC_DEBUG << "Movie::readFrame time=" << theCurrentTime << " src=" << get<ImageSourceTag>());
        DB(AC_DEBUG << "                 theIgnoreCurrentTime=" << theIgnoreCurrentTime);
        _myLastCurrentTime = theCurrentTime;

        if (!_myDecoder) {
            AC_ERROR << "Movie::readFrame not allowed before open";
            return;
        }
        // Check for playmode changes from javascript
        MoviePlayMode myPlayMode = MoviePlayMode(asl::getEnumFromString(get<PlayModeTag>(), MoviePlayModeStrings));
        if (myPlayMode != _myPlayMode) {
            setPlayMode(myPlayMode);
        }

        // Calculate next frame
        int myNextFrame;
        double myMovieTime;
        switch (_myPlayMode) {
            case PLAY_MODE_PAUSE:
                // next frame from currentframe attribute in movie node
                myNextFrame = get<CurrentFrameTag>();
                if (get<FrameCountTag>() != -1) {
                    myNextFrame %= get<FrameCountTag>();
                }
                while (myNextFrame < 0) {
                    if (get<FrameCountTag>() == -1) {
                        std::string myErrorMsg = string("Movie: ") + get<NameTag>() + " has negative currentframe and a invalid framecount, " + 
                                                 "trying a wraparound with invalid framecount will fail-> calling " + 
                                                 "explicit ensureFramecount will help";
                        ASSURE_MSG(get<FrameCountTag>() != -1, myErrorMsg.c_str());
                        return;
                    }
                    myNextFrame += get<FrameCountTag>();
                }
                myMovieTime = getTimeFromFrame(myNextFrame);
                break;
            case PLAY_MODE_PLAY:
                if (theIgnoreCurrentTime) {
                    myMovieTime = 0;
                } else {
                    myMovieTime = _myDecoder->getMovieTime(theCurrentTime);
                }
                myNextFrame = (int)getFrameFromTime(myMovieTime);
                break;
            case PLAY_MODE_STOP:
                myNextFrame = _myLastDecodedFrame;
                return;
        }

        if (myNextFrame < 0) {
            setPlayMode(PLAY_MODE_STOP);
        }

        DB(AC_DEBUG << "Next Frame: " << myNextFrame << ", lastDecodedFrame: " << _myLastDecodedFrame << ", MovieTime: " << myMovieTime;)
        if (myNextFrame != _myLastDecodedFrame) {
            MAKE_SCOPE_TIMER(Movie_readFrame);
            double myDecodedTime = decodeFrame(myMovieTime, myNextFrame);
            /*if (!asl::almostEqual(myDecodedTime, myMovieTime, 0.04)) {
                AC_WARNING << "Decoded Time=" << myDecodedTime << " differs from Movie Time=" << myMovieTime << ". Delta=" << myMovieTime-myDecodedTime;
            }*/
        }

        // check for eof in the decoder
        if (_myDecoder->getEOF()) {
            AC_DEBUG << "Movie has EOF, loopCount=" << _myCurrentLoopCount;
            _myDecoder->setEOF(false);
            if (get<LoopCountTag>() == 0 || ++_myCurrentLoopCount < get<LoopCountTag>()) {
                restart(theCurrentTime);
            } else {
                setPlayMode(PLAY_MODE_STOP);
            }
        }
    }

    bool Movie::getMovieTime(double & theTime) const {
        //DK my workaround for not knowing how to const_cast the asl::Ptr itself
        MovieDecoderBase & myDecoder = const_cast<MovieDecoderBase&>(*_myDecoder);
        theTime = myDecoder.getMovieTime(_myLastCurrentTime);
        AC_DEBUG << "Movie::getMovieTime systime " << _myLastCurrentTime
                 << " got " << theTime;
        return true;
    }

    bool Movie::getDecoderName(std::string & theName) const {
        AC_DEBUG << "Movie::getDecoderName";
        MovieDecoderBase & myDecoder = const_cast<MovieDecoderBase&>(*_myDecoder);
        theName = myDecoder.getName();
        AC_DEBUG << "Movie::getDecoderName got " << theName;
        return true;
    }

    void Movie::load() {}

    void Movie::load(asl::PackageManager & thePackageManager) {
        /*
         * XXX
         * decide here wether you want to use the stream interface of thePackageManager
         * advantage: you could load mpegs which are in a zip file known to thePackageManager
         * disadvantages: 1) big files (> 200MB) are AFAIK currently not handled by thePackageManager
         *                2) loadStream currently does not fallback to loadFile for WMV and alike
         * DK
         */
#if 0
        loadStream( thePackageManager.openFile(get<ImageSourceTag>()), get<ImageSourceTag>());
#else
        loadFile( thePackageManager.searchFile(get<ImageSourceTag>()) );
#endif
    }

    MovieDecoderBasePtr Movie::getDecoder(const std::string theFilename) {
        MovieDecoderBasePtr myDecoder = DecoderManager::get().findDecoder<MovieDecoderBase>(theFilename);

        const std::string & myDecoderHint = get<DecoderHintTag>();
        if (myDecoderHint != "") {
            std::vector<MovieDecoderBasePtr> myDecoders;
            myDecoders = DecoderManager::get().findAllDecoders<MovieDecoderBase>(theFilename);
            std::vector<MovieDecoderBasePtr>::iterator it;
            bool foundDecoderFlag = false;
            for(it = myDecoders.begin(); it != myDecoders.end(); ++it) {
                AC_DEBUG << "possible decoder " << (*it)->getName();
                if ((*it)->getName() == myDecoderHint) {
                    myDecoder = (*it);
                    foundDecoderFlag = true;
                    break;
                }
            }
            if (!foundDecoderFlag) {
                // we did not find a decoder for the decoderhint, plug it and try to use it
                asl::PlugInBasePtr myPlugIn = asl::PlugInManager::get().getPlugIn(myDecoderHint);
                if (IDecoderPtr myDecoderPlug = dynamic_cast_Ptr<IDecoder>(myPlugIn)) {
                    AC_INFO << "Plug: " << myDecoderHint << ": as Decoder" << endl;
                    DecoderManager::get().addDecoder(myDecoderPlug);
                    myDecoder = dynamic_cast_Ptr<MovieDecoderBase>(myDecoderPlug);
                } else {
                    throw MovieException(std::string("Unable to plug decoder: ") + myDecoderHint, PLUS_FILE_LINE);
                }
            }
        }
//        AC_INFO << "using decoder " << myDecoder->getName() << " for decoding " << theFilename << " hint " << myDecoderHint;
        return myDecoder;
    }

    void
    Movie::load(const std::string & theTexturePath) {
        loadFile( asl::searchFile(get<ImageSourceTag>(), theTexturePath) );
    }

    void
    Movie::loadStream(asl::Ptr<asl::ReadableStream> theSource, const std::string theUrl) {
        AC_INFO << "Movie::loadStream " << theUrl;
        MovieDecoderBasePtr myDecoder = getDecoder(theUrl);
        if (!myDecoder) {
            throw MovieException(string("Sorry, could not find a streamable decoder for: ") + theUrl, PLUS_FILE_LINE);
        }
        _myDecoder = myDecoder->instance();

        _myDecoder->initialize(this);
        _myDecoder->load(theSource, theUrl);

        postLoad();
    }


    void
    Movie::loadFile(const std::string & theUrl) {

        const std::string & mySourceFile = get<ImageSourceTag>();
        if (_myDecoder) {
            _myDecoder->stopMovie();
        }
        //set<CurrentFrameTag>(0);
        _myLastDecodedFrame = UINT_MAX;
        _myCurrentLoopCount = 0;

        _myPlayMode = PLAY_MODE_STOP;

        // if imagesource is an url do not take the packetmanager or searchfile new url

        string myFilename;
        if (mySourceFile.find("://") != string::npos) {
            myFilename = mySourceFile;
        } else {
            myFilename = theUrl;
        }
        if (myFilename.empty()) {
            AC_ERROR << "Unable to find url='" << theUrl << "'" << ", src='" << mySourceFile << "'";
            return;
        }
        AC_INFO << "Movie::loadFile " << (void*)this << " filename=" << myFilename;

        // First: Look for registered decoders that could handle the source
        MovieDecoderBasePtr myDecoder = getDecoder(myFilename);

        if (!myDecoder) {
            // Second: Try m60, by extension
            string myFileExtension = asl::toLowerCase(asl::getExtension(myFilename));
            if (myFileExtension == "m60") {
                _myDecoder = MovieDecoderBasePtr(new M60Decoder());
            } else {
                throw MovieException(std::string("Sorry, could not find decoder for: ") + myFilename, PLUS_FILE_LINE);
            }
        } else {
            _myDecoder = myDecoder->instance();
        }
        _myDecoder->initialize(this);
        _myDecoder->load(myFilename);
        postLoad();
    }

    void Movie::postLoad() {

        /*
         * UH: theUrl is mangled by the PackageManager and is not necessarily the same as the ImageSourceTag
         * this leads to multiple load() calls by the TextureManager since reloadRequired checks
         * _myLoadedFilename against ImageSourceTag and finds them to differ.
         */
        _myLoadedFilename = get<ImageSourceTag>();
        if (get<ImageResizeTag>() == IMAGE_RESIZE_PAD) { 
            float myXResize = float(get<ImageWidthTag>()) / asl::nextPowerOfTwo(get<ImageWidthTag>());
            float myYResize = float(get<ImageHeightTag>()) / asl::nextPowerOfTwo(get<ImageHeightTag>());

            asl::Matrix4f myMatrix;
            myMatrix.makeScaling(asl::Vector3f(myXResize, myYResize, 1.0f));
            set<ImageMatrixTag>(myMatrix);
        }        
    }

    bool
    Movie::reloadRequired(){
        bool rc = !_myDecoder || _myLoadedFilename != get<ImageSourceTag>();
        AC_TRACE << "Movie::reloadRequired " << rc
                 << " with _myLoadedFilename=" << _myLoadedFilename
                 << " ImageSourceTag=" << get<ImageSourceTag>();
        if (_myLoadedFilename != get<ImageSourceTag>()) {
            set<FrameCountTag>(-1);            
        }                 
        return rc;
    }
}
