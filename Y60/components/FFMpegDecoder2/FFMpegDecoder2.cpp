//=============================================================================
// Copyright (C) 2004-2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "FFMpegDecoder2.h"
#include "FFMpegURLProtocol.h"

#include <y60/SoundManager.h>
#include <asl/Ptr.h>
#include <asl/Auto.h>
#include <asl/Pump.h> //must come before Assure.h
#include <asl/Block.h>
#include <asl/Logger.h>
#include <asl/Assure.h>
#include <asl/file_functions.h>

#include <iostream>

#define DB(x) //x
#define DB2(x) //x

using namespace std;
using namespace asl;

#define USE_RGBA 0
#define USE_GRAY 1

const long SEMAPHORE_TIMEOUT = asl::ThreadSemaphore::WAIT_INFINITE; //1000 * 60 * 1;

extern "C"
EXPORT asl::PlugInBase * y60FFMpegDecoder2_instantiatePlugIn(asl::DLHandle myDLHandle) {
	return new y60::FFMpegDecoder2(myDLHandle);
}

namespace y60 {

    asl::Block FFMpegDecoder2::_myResampledSamples(AVCODEC_MAX_AUDIO_FRAME_SIZE);
    asl::Block FFMpegDecoder2::_mySamples(AVCODEC_MAX_AUDIO_FRAME_SIZE);

    FFMpegDecoder2::FFMpegDecoder2(asl::DLHandle theDLHandle) :
        PlugInBase(theDLHandle), AsyncDecoder(), PosixThread(),
        _myFormatContext(0), _myFrame(0),
        _myVStreamIndex(-1), _myVStream(0), _myStartTimestamp(0),
        _myAStreamIndex(-1), _myAStream(0),
        _mySeekTimestamp(AV_NOPTS_VALUE), _myLastSeekTimestamp(AV_NOPTS_VALUE),
        _myEOFVideoTimestamp(INT_MIN), _myLastAudioTimeStamp(0),
        _myNextPacketTimestamp(0),
        _myTimePerFrame(0), _myLineSizeBytes(0), _myDestinationPixelFormat(0),
        _myFrameCache(SEMAPHORE_TIMEOUT), _myFrameRecycler(SEMAPHORE_TIMEOUT),
        _myResampleContext(0)
    {
    }

    FFMpegDecoder2::~FFMpegDecoder2() {
        closeMovie();
        if (_myResampleContext) {
            audio_resample_close(_myResampleContext);
            _myResampleContext = 0;
        }
    }

    asl::Ptr<MovieDecoderBase> FFMpegDecoder2::instance() const {
        return asl::Ptr<MovieDecoderBase>(new FFMpegDecoder2(getDLHandle()));
    }

    std::string
    FFMpegDecoder2::canDecode(const std::string & theUrl, asl::ReadableStream * theStream) {
        if (asl::toLowerCase(asl::getExtension(theUrl)) == "mpg" ||
            asl::toLowerCase(asl::getExtension(theUrl)) == "m2v" ||
            asl::toLowerCase(asl::getExtension(theUrl)) == "avi" ||
            asl::toLowerCase(asl::getExtension(theUrl)) == "mov" ||
            asl::toLowerCase(asl::getExtension(theUrl)) == "mpeg") {
            AC_INFO << "FFMpegDecoder2 can decode :" << theUrl << endl;
            return MIME_TYPE_MPG;
        } else {
            AC_INFO << "FFMpegDecoder2 can not decode :" << theUrl << " responsible for extensions mpg and m2v" << endl;
            return "";
        }
    }


    bool FFMpegDecoder2::addAudioPacket(const AVPacket & thePacket) {
        // decode audio
        int myBytesDecoded = 0; // decompressed sample size in bytes
        unsigned char* myData = thePacket.data;
        unsigned myDataLen = thePacket.size;
        double myTime = thePacket.dts / (double)_myTimeUnitsPerSecond;

        AC_TRACE << "FFMpegDecoder2::addAudioPacket()";
        while (myDataLen > 0) {
#if LIBAVCODEC_BUILD >= 0x5100
            int myLen = avcodec_decode_audio(_myAStream->codec,
                (int16_t*)_mySamples.begin(), &myBytesDecoded, myData, myDataLen);
#else
            int myLen = avcodec_decode_audio(&_myAStream->codec,
                (int16_t*)_mySamples.begin(), &myBytesDecoded, myData, myDataLen);
#endif
            if (myLen < 0 || myBytesDecoded < 0) {
                AC_WARNING << "av_decode_audio error";
                break;
            }
#if LIBAVCODEC_BUILD >= 0x5100
            int myNumChannels = _myAStream->codec->channels;
#else
            int myNumChannels = _myAStream->codec.channels;
#endif
            int numFrames = myBytesDecoded/(getBytesPerSample(SF_S16)*myNumChannels);
            AC_TRACE << "FFMpegDecoder2::decode(): Frames per buffer= " << numFrames;
            // queue audio sample
            AudioBufferPtr myBuffer;
            if (_myResampleContext) {
                numFrames = audio_resample(_myResampleContext,
                        (int16_t*)(_myResampledSamples.begin()),
                        (int16_t*)(_mySamples.begin()),
                        numFrames);
                myBuffer = Pump::get().createBuffer(numFrames);
                myBuffer->convert(_myResampledSamples.begin(), SF_S16, myNumChannels);
            } else {
                myBuffer = Pump::get().createBuffer(numFrames);
                myBuffer->convert(_mySamples.begin(), SF_S16, myNumChannels);
            }
            _myAudioSink->queueSamples(myBuffer);

            myData += myLen;
            myDataLen -= myLen;
            AC_TRACE << "data left " << myDataLen << " read " << myLen;

            AC_DEBUG << "decoded audio time=" << myTime;
            _myLastAudioTimeStamp = myTime;
        } // while

        // adjust volume
        float myVolume = getMovie()->get<VolumeTag>();
        if (!asl::almostEqual(Pump::get().getVolume(), myVolume)) {
            Pump::get().setVolume(myVolume);
        } //if
        return true;
    }

    bool FFMpegDecoder2::addVideoPacket(const AVPacket & thePacket) {
        int frameFinished = 0;
        unsigned char* myData = thePacket.data;
        int myDataLen = thePacket.size;
        double myTime = thePacket.dts * _myTimeUnitsPerSecond; 

        AC_TRACE << "decoding @ " << myTime << " size " << myDataLen;
        while (frameFinished == 0 && myDataLen > 0) {
#if LIBAVCODEC_BUILD >= 0x5100
            int myLen = avcodec_decode_video(_myVStream->codec,
                _myFrame, &frameFinished, myData, myDataLen);
#else
            int myLen = avcodec_decode_video(&_myVStream->codec,
                _myFrame, &frameFinished, myData, myDataLen);
#endif
            AC_TRACE << "avcodec_decode_video returned " << myLen << " got pic " << frameFinished;
            if (myLen < 0) {
                AC_ERROR << "av_decode_video error";
                break;
            }
            myData += myLen;
            myDataLen -= myLen;
        }
        if (frameFinished == 0) {
            AC_TRACE << "Frame unfinished";
            return false;
        }
        // [TS] myPacket.dts seems to be b0rken or at least off by 1 frame on our
        // mpeg files
        _myNextPacketTimestamp += _myTimePerFrame;
        addCacheFrame(_myFrame, thePacket.dts-_myTimePerFrame);
        return true;
    }

    bool FFMpegDecoder2::updateCache() {
        AVPacket myPacket;
        memset(&myPacket, 0, sizeof(myPacket));
        // until a frame is found or eof
        while (true) {
            AC_TRACE << "updateCache";

            if (av_read_frame(_myFormatContext, &myPacket) < 0) {
                av_free_packet(&myPacket);

                // Remember the end of file timestamp
                if (_myEOFVideoTimestamp == INT_MIN) {
                    _myEOFVideoTimestamp = _myNextPacketTimestamp;
                }

                /* some codecs, such as MPEG, transmit the I and P frame with a
                 * latency of one frame. You must do the following to have a
                 * chance to get the last frame of the video
                 */
                int frameFinished = 0;
#if LIBAVCODEC_BUILD >= 0x5100
                int myLen = avcodec_decode_video(_myVStream->codec, _myFrame, &frameFinished, NULL, 0);
#else
                int myLen = avcodec_decode_video(&_myVStream->codec, _myFrame, &frameFinished, NULL, 0);
#endif
                if (frameFinished) {
		            AC_TRACE << "add frame";
                    addCacheFrame(_myFrame, _myEOFVideoTimestamp);
                    continue;
                } else {
					AC_TRACE << "frame not ready";
                    return false;
                }
            }
            double myTime = myPacket.dts * _myTimeUnitsPerSecond;

            if (myPacket.stream_index == _myVStreamIndex) {
                AC_DEBUG << "video time=" << myTime;
                // If no frame has been added, we continue here until we have added one
                // If we added a video Frame, we break out of the loop and return true.
                if (!addVideoPacket(myPacket)) {
                    continue;
                } else {
                    return true;
                }
            } else if (myPacket.stream_index == _myAStreamIndex && getAudioFlag()) {
                addAudioPacket(myPacket);
            } // else if (myPacket.stream_index == _myAStreamIndex)
        }
        av_free_packet(&myPacket);
        return true;
    }

    void FFMpegDecoder2::convertFrame(AVFrame* theFrame, unsigned char* theBuffer) {
        if (!theFrame) {
            AC_ERROR << "FFMpegDecoder::convertFrame invalid AVFrame";
            return;
        }

        AVPicture myDestPict;
        myDestPict.data[0] = theBuffer;
        myDestPict.data[1] = theBuffer+1;
        myDestPict.data[2] = theBuffer+2;

        myDestPict.linesize[0] = _myLineSizeBytes;
        myDestPict.linesize[1] = _myLineSizeBytes;
        myDestPict.linesize[2] = _myLineSizeBytes;

#if LIBAVCODEC_BUILD >= 0x5100
        AVCodecContext * myVCodec = _myVStream->codec;
#else
        AVCodecContext * myVCodec = &_myVStream->codec;
#endif
        img_convert(&myDestPict, _myDestinationPixelFormat,
                    (AVPicture*)theFrame, myVCodec->pix_fmt,
                    myVCodec->width, myVCodec->height);
    }

    void FFMpegDecoder2::copyFrame(FrameCache::VideoFramePtr theVideoFrame,
                                   dom::ResizeableRasterPtr theTargetRaster)
    {
        theTargetRaster->resize(getFrameWidth(), getFrameHeight());
        memcpy(theTargetRaster->pixels().begin(), theVideoFrame->getBuffer(), theTargetRaster->pixels().size());
    }

    void FFMpegDecoder2::createCache() {

        AC_DEBUG << "FFMpegDecoder2::createCache PIX_FMT=" << _myDestinationPixelFormat;

        _myFrameRecycler.clear();
        _myFrameRecycler.reset();

        // create cache
        unsigned myBufferSize = 0;
        switch (_myDestinationPixelFormat) {
        case PIX_FMT_RGBA32:
            myBufferSize = getFrameWidth() * getFrameHeight() * 4;
            break;
        case PIX_FMT_GRAY8:
            myBufferSize = getFrameWidth() * getFrameHeight() * 1;
            break;
        default:
            myBufferSize = getFrameWidth() * getFrameHeight() * 3;
            break;
        }
        while (_myFrameRecycler.size() < FRAME_CACHE_SIZE) {
            _myFrameRecycler.push_back(FrameCache::VideoFramePtr(new FrameCache::VideoFrame(myBufferSize)));
        }
        _myFrameCache.push_back(FrameCache::VideoFramePtr(new FrameCache::VideoFrame(myBufferSize)));
    }

    void
    FFMpegDecoder2::load(Ptr<ReadableStream> theSource, const string & theFilename) {
        string theStreamID = string("acstream://") + theFilename;
        registerStream(theStreamID, theSource);
        load(theStreamID);
    }

    void
    FFMpegDecoder2::load(const std::string & theFilename) {
        asl::AutoLocker<FFMpegDecoder2> myLock(*this);

        // register all formats and codecs
        static bool avRegistered = false;
        if (!avRegistered) {
            AC_INFO << "FFMpegDecoder2::load " << LIBAVCODEC_IDENT;
            av_log_set_level(AV_LOG_ERROR);
            av_register_all();
            avRegistered = true;
        }

        if (av_open_input_file(&_myFormatContext, theFilename.c_str(), 0, FFM_PACKET_SIZE, 0) < 0) {
            throw FFMpegDecoder2Exception(std::string("Unable to open input file: ") + theFilename, PLUS_FILE_LINE);
        }

        if (av_find_stream_info(_myFormatContext) < 0) {
            throw FFMpegDecoder2Exception(std::string("Unable to find stream info: ") + theFilename, PLUS_FILE_LINE);
        }

        // find video/audio streams
        for (unsigned i = 0; i < _myFormatContext->nb_streams; ++i) {
#if LIBAVCODEC_BUILD >= 0x5100
            int myCodecType =  _myFormatContext->streams[i]->codec->codec_type;
#else
            int myCodecType =  _myFormatContext->streams[i]->codec.codec_type;
#endif
            if (_myVStreamIndex == -1 && myCodecType == CODEC_TYPE_VIDEO) {
                _myVStreamIndex = i;
                _myVStream = _myFormatContext->streams[i];
            }
            else if (_myAStreamIndex == -1 && myCodecType == CODEC_TYPE_AUDIO) {
                _myAStreamIndex = i;
                _myAStream = _myFormatContext->streams[i];
            }
        }
#if LIBAVCODEC_BUILD >= 0x5100        
        _myTimeUnitsPerSecond = (int64_t)(1/ av_q2d(_myVStream->time_base));
#else
        _myTimeUnitsPerSecond = (int64_t)AV_TIME_BASE;
#endif
        if (_myVStream) {
            setupVideo(theFilename);
        } else {
            AC_INFO << "FFMpegDecoder2::load " << theFilename << " no video stream found";
            _myVStream = 0;
            _myVStreamIndex = -1;
        }
        if (_myAStream && getAudioFlag()) {
            setupAudio(theFilename);
        } else {
            AC_INFO << "FFMpegDecoder2::load " << theFilename << " no audio stream found or disabled";
            _myAudioSink = HWSampleSinkPtr(0);
            _myAStream = 0;
            _myAStreamIndex = -1;
        }
        _myEOFVideoTimestamp = INT_MIN;
        createCache();
#if 0
        // seek to start
#if (LIBAVCODEC_BUILD < 4738)
        int myResult = av_seek_frame(_myFormatContext, -1, _myStartTimestamp);
#else
        int myResult = av_seek_frame(_myFormatContext, -1, _myStartTimestamp, AVSEEK_FLAG_BACKWARD);
#endif
#endif

    }

    double
    FFMpegDecoder2::readFrame(double theTime, unsigned theFrame, dom::ResizeableRasterPtr theTargetRaster) {
        ASSURE(!getEOF());
        if (theTargetRaster == 0) {
            throw FFMpegDecoder2Exception("TargetRasterPtr is null.", PLUS_FILE_LINE);
        }
        try {
            FrameCache::VideoFramePtr myVideoFrame(0);
            if (getPlayMode() != y60::PLAY_MODE_PLAY) {
                return theTime;
            }
            int64_t myBaseTime = (int64_t)(theTime * _myTimeUnitsPerSecond);
            int64_t myFrameTimestamp = _myStartTimestamp + (int64_t)(myBaseTime);
            AC_DEBUG << "Time=" << theTime << " - Reading FrameTimestamp: " << myFrameTimestamp << " from Cache.";
            {
                for (;;) {
                    myVideoFrame = _myFrameCache.pop_front();
                    int64_t myTimeDiff = myFrameTimestamp - myVideoFrame->getTimestamp();
                    AC_TRACE << "TimeDiff: " << myTimeDiff;
    				if (myTimeDiff <= 0 || _myFrameCache.size() == 0) {
                        _myFrameCache.push_front(myVideoFrame);
                        break;
                    } else {
                        _myFrameRecycler.push_back(myVideoFrame);
                    }
                }
            }
            // XXX Handle EOF
			AC_TRACE << "readFrame trying to acquire lock";
            asl::AutoLocker<FFMpegDecoder2> myLock(*this);
			AC_TRACE << "readFrame acquired lock";
            if (_myReadEOF && _myFrameCache.size() <= 1) {
                AC_TRACE << "Readframe: EOF Write down";
                _myReadEOF = false;
                AC_DEBUG << "Setting Movie to EOF";
                setEOF(true);
				AC_TRACE << "readFrame (1) release lock";
                return theTime;
            }

            /*
            if (myVideoFrame && myVideoFrame->getTimestamp() != myFrameTimestamp) {
            // not found, seek to frame
            //seekToFrame(theFrame);
            yield();
            }
            */
            // convert frame
            theTargetRaster->resize(getFrameWidth(), getFrameHeight());
            if (myVideoFrame) {
                copyFrame(myVideoFrame, theTargetRaster);
            } else {
                memset(theTargetRaster->pixels().begin(), 0, theTargetRaster->pixels().size());
            }
            AC_TRACE << "Readframe ending.";
            getMovie()->set<CacheSizeTag>(_myFrameCache.size());

			AC_TRACE << "readFrame (2) release lock";
        } catch (asl::ThreadSemaphore::ClosedException &) {
            AC_WARNING << "Semaphore Destroyed while in readframe";
            return theTime;
        } catch (FrameCache::TimeoutException &) {
            AC_FATAL << "Semaphore Timeout";
            return theTime;
        }
        return theTime;
    }

    void FFMpegDecoder2::startMovie(double theStartTime) {
        AC_DEBUG << "startMovie, time: " << theStartTime << ". Resetting locks";
        _myNextPacketTimestamp = 0;

        asl::AutoLocker<FFMpegDecoder2> myLock(*this);
        _myFrameCache.clear();
        createCache();
        // XXX unblock locked ffmpeg-thread.
        _myReadEOF = false;
        if (_myAudioSink) {
            _myAudioSink->stop();
        }
        _myCachingFlag = true;
        _mySeekTimestamp = AV_NOPTS_VALUE;
        _myLastSeekTimestamp = AV_NOPTS_VALUE;
        _myEOFVideoTimestamp = INT_MIN;
        _myLastAudioTimeStamp = 0;
        _myNextPacketTimestamp = 0;

        // seek to start
#if (LIBAVCODEC_BUILD < 4738)
        int myResult = av_seek_frame(_myFormatContext, -1, _myStartTimestamp);
#else
        int myResult = av_seek_frame(_myFormatContext, -1, _myStartTimestamp, AVSEEK_FLAG_BACKWARD);
#endif
#if LIBAVCODEC_BUILD >= 0x5100
        avcodec_flush_buffers(_myVStream->codec);
#else
        avcodec_flush_buffers(&_myVStream->codec);
#endif
        updateCache();

        _myState = RUN;
        if (!isActive()) {
            AC_TRACE << "Forking FFMpegDecoder Thread";
            PosixThread::fork();
        } else {
            AC_INFO << "Thread already running. No forking.";
        }
        AC_DEBUG << "Movie starting. _myStartTimestamp is " << _myStartTimestamp;
        AsyncDecoder::startMovie(theStartTime);
    }

    void FFMpegDecoder2::resumeMovie(double theStartTime) {
        AC_DEBUG << "resumeMovie, time: " << theStartTime << ". Resetting locks";
        asl::AutoLocker<FFMpegDecoder2> myLock(*this);
        _myNextPacketTimestamp = 0;
        _myState = RUN;
        if (!isActive()) {
            AC_TRACE << "Forking FFMpegDecoder Thread";
            PosixThread::fork();
        } else {
            AC_INFO << "Thread already running. No forking.";
        }
        AC_DEBUG << "Movie resuming. _myStartTimestamp is " << _myStartTimestamp;
        AsyncDecoder::resumeMovie(theStartTime);
    }

    void FFMpegDecoder2::stopMovie() {
        AC_DEBUG << "stopMovie";
		lock();
        _myState = STOP;
		unlock();
        // Wake up sleeping decoder thread
        if (isActive()) {
            AC_TRACE << "Joining FFMpegDecoder Thread";
            _myFrameRecycler.close();
            join();
        }
        AsyncDecoder::stopMovie();
    }

    void
    FFMpegDecoder2::closeMovie() {
        AC_DEBUG << "closeMovie";
        // stop thread
        stopMovie();

        // codecs
        if (_myVStream) {
#if LIBAVCODEC_BUILD >= 0x5100
            avcodec_close(_myVStream->codec);
#else
            avcodec_close(&_myVStream->codec);
#endif
            _myVStreamIndex = -1;
            _myVStream = 0;
        }
        if (_myAStream) {
#if LIBAVCODEC_BUILD >= 0x5100
            avcodec_close(_myAStream->codec);
#else
            avcodec_close(&_myAStream->codec);
#endif
            _myAStreamIndex = -1;
            _myAStream = 0;
        }

        av_close_input_file(_myFormatContext);
        _myFormatContext = 0;
        AsyncDecoder::closeMovie();
    }

    void FFMpegDecoder2::run() {

        AC_TRACE << "run starting";
        if (_myVStream == 0 && _myAStream == 0) {
			AC_WARNING << "Neither audio nor video stram in FFMpegDecoder2::run";
            return;
        }
        _myReadEOF = false;
        double myFrameRate = getFrameRate();

        int64_t mySeekTimestamp = AV_NOPTS_VALUE;

        // decoder loop
        while (_myState != STOP) {
			AC_TRACE << "FFMpegDecoder2::loop";
			//AC_DEBUG << "Loop";

            // do nothing...
            if (_myState != RUN) {
                asl::msleep(10);
                yield();
				AC_TRACE << "FFMpegDecoder2::run : state =! run, yielding and continue";
                continue;
            }
			AC_TRACE << "FFMpeg-Tread acquiring lock";
	        lock();
			AC_TRACE << "FFMpeg-Tread lock acquired.";
            if (_myReadEOF) {
				unlock();
                asl::msleep(500);
				AC_TRACE << "EOF read.";
                continue;
            }
			unlock();

            // seek to timestamp
            if (_mySeekTimestamp != AV_NOPTS_VALUE) {
                AC_INFO << "seeking to=" << _mySeekTimestamp;
#if (LIBAVCODEC_BUILD < 4738)
                int myResult = av_seek_frame(_myFormatContext, -1, _mySeekTimestamp);
#else
                int myResult = av_seek_frame(_myFormatContext, -1, _mySeekTimestamp, AVSEEK_FLAG_BACKWARD);
#endif
                if (myResult < 0) {
                    AC_WARNING << "FFMpegDecoder2::run unable to seek to " << _mySeekTimestamp;
                }
                _mySeekTimestamp = AV_NOPTS_VALUE;
            }
            AC_TRACE << "Updating cache";
            try {
                if (!updateCache()) {
                    unsigned myLastFrame = asl::round(myFrameRate * 
                            (_myEOFVideoTimestamp - _myStartTimestamp) / _myTimeUnitsPerSecond);
                    AC_TRACE << "EOF in Decoder, StartTimeStamp: " << _myStartTimestamp 
                            << ", EOFTimestamp: " << _myEOFVideoTimestamp << ", Lastframe: " 
                            << myLastFrame;
                    if (getFrameCount() == INT_MAX) {
                        getMovie()->set<FrameCountTag>(myLastFrame + 1);
                        AC_INFO << "FFMpegDecoder::readFrame set framecount=" << getFrameCount();
                    }
					lock();
                    _myReadEOF = true;
					unlock();
					AC_TRACE << "EOF Yielding Thread.";
                    yield();
                }
            } catch (asl::ThreadSemaphore::ClosedException &) {
                AC_WARNING << "Semaphore destroyed while in updateCache. Terminating Thread.";
                // Be sure we don't have a lock acquired here.
                return;
            }

			AC_TRACE << "start audio " << getAudioFlag();
            // Start playback?
            double myStartTime = (FRAME_CACHE_SIZE/2) / myFrameRate;
            if (_myAStream && getAudioFlag() && _myCachingFlag &&
                (_myFrameCache.size() >= FRAME_CACHE_SIZE/2 /*|| _myLastAudioTimeStamp >= myStartTime*/))
            {
                AC_INFO << "Start Audio. Cache: " << _myFrameCache.size();
                _myCachingFlag = false;
                _myAudioSink->play();
            } else {
                AC_DEBUG << "Caching, Cache: " << _myFrameCache.size() << ", StartTime: " << myStartTime;
            }
			AC_TRACE << "end of loop";
        }
        AC_TRACE << "FFMpegDecoder2::run terminating";
    }

    int64_t FFMpegDecoder2::getTimestampFromFrame(unsigned theFrame) {
        return _myStartTimestamp + (int64_t)((theFrame * _myTimeUnitsPerSecond)/getFrameRate());
    }
    
    unsigned FFMpegDecoder2::getFrameFromTimestamp(int64_t theTimestamp) {
        return (unsigned)(((theTimestamp - _myStartTimestamp) * getFrameRate())/
                (double)_myTimeUnitsPerSecond);
    }

    int64_t FFMpegDecoder2::seekToFrame(unsigned theFrame) {
        int64_t myTimestamp = getTimestampFromFrame(theFrame);
        seekToTimestamp(myTimestamp);
        return myTimestamp;
    }

    void FFMpegDecoder2::seekToTimestamp(int64_t theTimestamp) {
        _myLastSeekTimestamp = _mySeekTimestamp;
        _mySeekTimestamp = theTimestamp;
        if (_myVStream) {
            AC_TRACE << "flushing video codec buffers";
#if LIBAVCODEC_BUILD >= 0x5100
            avcodec_flush_buffers(_myVStream->codec);
#else
            avcodec_flush_buffers(&_myVStream->codec);
#endif
        }
        if (_myAudioSink) {
            _myAudioSink->stop();
        }
    }

    void FFMpegDecoder2::setupVideo(const std::string & theFilename) {
#if LIBAVCODEC_BUILD >= 0x5100
        AVCodecContext * myVCodec = _myVStream->codec;
#else
        AVCodecContext * myVCodec = &_myVStream->codec;
#endif

        // open codec
        AVCodec * myCodec = avcodec_find_decoder(myVCodec->codec_id);
        if (!myCodec) {
            throw FFMpegDecoder2Exception(std::string("Unable to find video codec: ") + theFilename, PLUS_FILE_LINE);
        }
        if (avcodec_open(myVCodec, myCodec) < 0 ) {
            throw FFMpegDecoder2Exception(std::string("Unable to open video codec: ") + theFilename, PLUS_FILE_LINE);
        }

        Movie * myMovie = getMovie();
        AC_TRACE << "PF=" << myMovie->get<ImagePixelFormatTag>();
        switch (myMovie->getPixelEncoding()) {
        case y60::RGBA:
        case y60::BGRA:
            AC_TRACE << "Using RGBA pixels";
            myMovie->set<ImagePixelFormatTag>(asl::getStringFromEnum(y60::RGBA, PixelEncodingString));
            _myDestinationPixelFormat = PIX_FMT_RGBA32;
            break;
        case y60::ALPHA:
        case y60::GRAY:
            AC_TRACE << "Using GRAY pixels";
            _myDestinationPixelFormat = PIX_FMT_GRAY8;
            break;
        case y60::RGB:
        case y60::BGR:
        default:
            AC_TRACE << "Using BGR pixels";
            _myDestinationPixelFormat = PIX_FMT_BGR24;
            myMovie->set<ImagePixelFormatTag>(asl::getStringFromEnum(y60::BGR, PixelEncodingString));
            break;
        }

        // Setup size and image matrix
        int myWidth = myVCodec->width;
        int myHeight = myVCodec->height;

        myMovie->set<ImageWidthTag>(myWidth);
        myMovie->set<ImageHeightTag>(myHeight);

        float myXResize = float(myWidth) / asl::nextPowerOfTwo(myWidth);
        float myYResize = float(myHeight) / asl::nextPowerOfTwo(myHeight);

        asl::Matrix4f myMatrix;
        myMatrix.makeScaling(asl::Vector3f(myXResize, myYResize, 1.0f));
        myMovie->set<ImageMatrixTag>(myMatrix);

        /*
         * hack to correct wrong frame rates that seem to be generated
         * by some codecs
         */
        float myFPS;
#if LIBAVCODEC_BUILD >= 0x5100
        myFPS = (1.0f / av_q2d(myVCodec->time_base));
#else
        if (_myVStream->codec.frame_rate > 1000 && _myVStream->codec.frame_rate_base == 1) {
            _myVStream->codec.frame_rate_base = 1000;
        }
        myFPS = _myVStream->codec.frame_rate / (float) _myVStream->codec.frame_rate_base;
        if (myFPS > 1000.0f) {
            myFPS /= 1000.0f;
        }
#endif
        myMovie->set<FrameRateTag>(myFPS);

        if (_myVStream->duration == AV_NOPTS_VALUE) {
            AC_INFO << "FFMpegDecoder2::setupVideo() " << theFilename << " has no valid duration";
        }
        if (_myVStream->duration == AV_NOPTS_VALUE || _myVStream->duration <= 0) {
            AC_WARNING << "FFMpegDecoder::load '" << theFilename 
                    << "' contains no valid duration";
            myMovie->set<FrameCountTag>(INT_MAX);
        } else {
	        myMovie->set<FrameCountTag>(int(myFPS * (_myVStream->duration / (double) _myTimeUnitsPerSecond)));
        }
        AC_INFO << "FFMpegDecoder2::setupVideo() " << theFilename << " fps=" << getFrameRate() << " framecount=" << getFrameCount();

        // allocate frame for YUV data
        _myFrame = avcodec_alloc_frame();

/* DK:
 * disabled because it causes errors (spratzer) while looping
 *
 * i don't think this is the start time you should seek to.
 * reading and decoding afterwards leads to incomplete frames (returned by av_decode_video)
 *
        // Get first timestamp
        AVPacket myPacket;
        bool myEndOfFileFlag = (av_read_frame(_myFormatContext, &myPacket) < 0);
        //_myFirstTimeStamp = myPacket.dts;
        AC_TRACE << "FFMpegDecoder2::setupVideo() - First packet has timestamp: " << myPacket.dts;
        av_free_packet(&myPacket);
        avcodec_flush_buffers(&_myVStream->codec);

        // Get starttime
        if (_myVStream->start_time != AV_NOPTS_VALUE) {
            _myStartTimestamp = _myVStream->start_time;
        } else {
            _myStartTimestamp = 0;
        }
 */
        _myStartTimestamp = 0;

        AC_TRACE << "FFMpegDecoder2::setupVideo() - Start timestamp: " << _myStartTimestamp;
        _myTimePerFrame = (int64_t)(_myTimeUnitsPerSecond/getFrameRate());
        cerr << "_myTimePerFrame: " << _myTimePerFrame << endl;
        _myLineSizeBytes = getBytesRequired(getFrameWidth(), getPixelFormat());
        // Seek to beginning
    }

    void
    FFMpegDecoder2::setupAudio(const std::string & theFilename) {
#if LIBAVCODEC_BUILD >= 0x5100
        AVCodecContext * myACodec = _myAStream->codec;
#else
        AVCodecContext * myACodec = &_myAStream->codec;
#endif

        // open codec
        AVCodec * myCodec = avcodec_find_decoder(myACodec->codec_id);
        if (!myCodec) {
            throw FFMpegDecoder2Exception(std::string("Unable to find audio decoder: ") + theFilename, PLUS_FILE_LINE);
        }
        if (avcodec_open(myACodec, myCodec) < 0 ) {
            throw FFMpegDecoder2Exception(std::string("Unable to open audio codec: ") + theFilename, PLUS_FILE_LINE);
        }

        _myAudioSink = Pump::get().createSampleSink(theFilename);

        if (myACodec->sample_rate != Pump::get().getNativeSampleRate())
        {
            _myResampleContext = audio_resample_init(myACodec->channels,
                    myACodec->channels, Pump::get().getNativeSampleRate(),
                    myACodec->sample_rate);
        }
        AC_INFO << "FFMpegDecoder2::setupAudio() done. resampling " << (_myResampleContext != 0);

    }

    void FFMpegDecoder2::addCacheFrame(AVFrame* theFrame, int64_t theTimestamp) {
		AC_TRACE << "try to add frame at " << theTimestamp;
        try {
            FrameCache::VideoFramePtr myVideoFrame = _myFrameRecycler.pop_front();
            myVideoFrame->setTimestamp(theTimestamp);
            convertFrame(theFrame, myVideoFrame->getBuffer());
            _myFrameCache.push_back(myVideoFrame);
            AC_TRACE << "Added Frame, Timestamp: " 
                    << double(theTimestamp)/_myTimeUnitsPerSecond;
        } catch (FrameCache::TimeoutException &) {
            AC_FATAL << "Semaphore Timeout";
        }
    }

    void FFMpegDecoder2::fillCache(double theStartTime) {
        while (_myFrameCache.size() < FRAME_CACHE_SIZE) {
            try {
                if (!updateCache()) {
                    unsigned myLastFrame =  getFrameRate() * 
                            unsigned(asl::round((_myEOFVideoTimestamp - _myStartTimestamp) /
                                        double(_myTimeUnitsPerSecond)));
                    AC_INFO << "EOF in Decoder, StartTimeStamp: " << _myStartTimestamp << ", EOFTimestamp: " << _myEOFVideoTimestamp << ", Lastframe: " << myLastFrame;
                    if (getFrameCount() == INT_MAX) {
                        getMovie()->set<FrameCountTag>(myLastFrame + 1);
                        AC_INFO << "FFMpegDecoder::readFrame set framecount=" << getFrameCount();
                    }
                    _myReadEOF = true;
                }
            } catch (asl::ThreadSemaphore::ClosedException &) {
                AC_TRACE << "Semaphore destroyed while in updateCache. Terminating Thread.";
                // Be sure we don't have a lock acquired here.
                return;
            }
        }
    }
}
