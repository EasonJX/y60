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
//   $RCSfile: FFMpegDecoder.h,v $
//   $Author: ulrich $
//   $Revision: 1.4 $
//   $Date: 2005/04/01 10:12:21 $
//
//  ffmpeg movie decoder.
//
//=============================================================================

#include "DecoderContext.h"
#include "FFMpegDecoder.h"
#include "AudioFrame.h"
#include "FrameAnalyser.h"

#include <asl/Logger.h>
#include <asl/os_functions.h>
#include <asl/console_functions.h>

#ifdef WIN32
#pragma warning( disable : 4244 ) // Disable ffmpeg warning
#define EMULATE_INTTYPES
#endif
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#ifdef WIN32
#pragma warning( default : 4244 ) // Renable ffmpeg warning
#endif

using namespace std;
using namespace asl;

#define DB(x) //x

namespace y60 {

    DecoderContext::DecoderContext(const std::string & theFilename) : 
        _myVideoStream(0),
        _myAudioStream(0),
        _myVideoStreamIndex(-1),
        _myAudioStreamIndex(-1),
        _myEndOfFileTimestamp(DBL_MAX),
        _myFilename(theFilename)        
    {
        // register all formats and codecs
        static bool avRegistered = false;
        if (!avRegistered) {
            AC_INFO << "FFMpegDecoder avcodec version " << LIBAVCODEC_IDENT;
            av_log_set_level(AV_LOG_ERROR);
            av_register_all();
            avRegistered = true;
        }

        // open file
        if (av_open_input_file(&_myFormatContext, theFilename.c_str(), 0, FFM_PACKET_SIZE, 0) < 0) {
            throw FFMpegDecoderException(std::string("Unable to open input file: ") + theFilename, PLUS_FILE_LINE);
        }
        if (av_find_stream_info(_myFormatContext) < 0) {
            throw FFMpegDecoderException(std::string("Unable to find stream info: ") + theFilename, PLUS_FILE_LINE);
        }

        // find video/audio streams
        for (unsigned i = 0; i < _myFormatContext->nb_streams; ++i) {
            if (_myVideoStreamIndex == -1 && _myFormatContext->streams[i]->codec.codec_type == CODEC_TYPE_VIDEO) {
                _myVideoStreamIndex = i;
                _myVideoStream = _myFormatContext->streams[i];
            } else if (_myAudioStreamIndex == -1 && _myFormatContext->streams[i]->codec.codec_type == CODEC_TYPE_AUDIO) {
                _myAudioStreamIndex = i;
                _myAudioStream = _myFormatContext->streams[i];
            }
        }    

        // open codec
        AVCodec * myCodec = avcodec_find_decoder(_myVideoStream->codec.codec_id);
        if (!myCodec) {
            throw FFMpegDecoderException(std::string("Unable to find decoder: ") + theFilename, PLUS_FILE_LINE);
        }
        if (avcodec_open(&_myVideoStream->codec, myCodec) < 0) {
            throw FFMpegDecoderException(std::string("Unable to open codec: ") + theFilename, PLUS_FILE_LINE);
        }

        // Hack to correct wrong frame rates that seem to be generated
        // by some codecs
        if (_myVideoStream->codec.frame_rate > 1000 && _myVideoStream->codec.frame_rate_base == 1) {
            _myVideoStream->codec.frame_rate_base = 1000;
        }

        _myAdvanceFrame = avcodec_alloc_frame();

        string myFrameAnalyserString;
        if (asl::get_environment_var("Y60_FRAME_ANALYSER", myFrameAnalyserString)) {            
            FrameAnalyser myFrameAnalyser(_myFormatContext, _myVideoStream, _myVideoStreamIndex);
            myFrameAnalyser.run(asl::as<unsigned>(myFrameAnalyserString));
        }
    }

    DecoderContext::~DecoderContext() {
        if (_myVideoStream) {
            avcodec_close(&_myVideoStream->codec);
            _myVideoStreamIndex = -1;
            _myVideoStream = 0;
        }
        if (_myAudioStream) {
            avcodec_close(&_myAudioStream->codec);
            _myAudioStreamIndex = -1;
            _myAudioStream = 0;
        }
        av_free(_myAdvanceFrame);
        av_close_input_file(_myFormatContext);
        _myFormatContext = 0;
    }

    PixelFormat 
    DecoderContext::getPixelFormat() {
        if (_myVideoStream) {
            return _myVideoStream->codec.pix_fmt;
        } else {
            return PIX_FMT_NB;
        }
    }
    unsigned DecoderContext::getWidth() {
        return _myVideoStream->codec.width;
    }
    unsigned DecoderContext::getHeight() {
        return _myVideoStream->codec.height;
    }

    DecoderContext::FrameType
    DecoderContext::decode(AVFrame * theVideoFrame, AudioFrame * theAudioFrame) {
        DecoderContext::FrameType myFrameType = FrameTypeEOF;
        AVPacket myPacket;
        //memset(&myPacket, 0, sizeof(myPacket));
        
        int64_t myStartTime = 0;
        if (_myVideoStream->start_time != AV_NOPTS_VALUE) {
            myStartTime = _myVideoStream->start_time;
        } 

        // Read until complete frame read or end of file reached
        bool myEndOfFileFlag = false;
        while (!(myEndOfFileFlag = (av_read_frame(_myFormatContext, &myPacket) < 0))) {
            if (myPacket.stream_index == _myVideoStreamIndex) {
                int myFrameCompleteFlag = 0;
                int myLen = avcodec_decode_video(&_myVideoStream->codec, theVideoFrame, &myFrameCompleteFlag,
                                                 myPacket.data, myPacket.size);

                if (myLen < 0) {
                    AC_ERROR << "av_decode_video error";
                } else if (myLen < myPacket.size) {
                    AC_ERROR << "av_decode_video: Could not decode video in one step";
                }                

                if (myFrameCompleteFlag) { 
                    theVideoFrame->pts = myPacket.dts - myStartTime;
                    myFrameType = FrameTypeVideo;
                    break;
                }                 
            } else if (myPacket.stream_index == _myAudioStreamIndex && theAudioFrame) {
                int myLen = avcodec_decode_audio(&_myAudioStream->codec, (int16_t*)theAudioFrame->getSamples(), (int *)&theAudioFrame->_mySampleSize,
                    myPacket.data, myPacket.size);
                theAudioFrame->_myTimestamp = (myPacket.dts - myStartTime) / (double)AV_TIME_BASE;

                if (myLen < 0) {
                    AC_ERROR << "avcodec_decode_audio error";
                } else if (myLen < myPacket.size) {
                    AC_ERROR << "avcodec_decode_audio: Could not decode video in one step";
                }

                myFrameType = FrameTypeAudio;
                break;
            } 
        }        

        if (!myEndOfFileFlag) {
            av_free_packet(&myPacket);
        }
        return myFrameType;
    }

    void
    DecoderContext::seekToTime(double theTimestamp) {        
        int myFrameIndex = int(theTimestamp * _myVideoStream->r_frame_rate / _myVideoStream->r_frame_rate_base);        
        double myFrameTime = double(_myVideoStream->r_frame_rate_base) / _myVideoStream->r_frame_rate;

        DB(cerr << TTYYELLOW << "seekToTime: " << theTimestamp << " index: " << myFrameIndex << ENDCOLOR << endl;)
        if (theTimestamp > myFrameTime * 1.5) {
            // We only want to decode to the timestamp just before the searched frame
            theTimestamp -= myFrameTime * 1.5;
        } else {
            theTimestamp = 0;
        }
        
        // newer releases of ffmpeg may require a 4th argument to av_seek_frame
        #if LIBAVFORMAT_BUILD <= 4616
            int myResult = av_seek_frame(_myFormatContext, _myVideoStreamIndex, myFrameIndex);
        #else
            int myResult = av_seek_frame(_myFormatContext, _myVideoStreamIndex, myFrameIndex, 0);
        #endif

        if (myResult < 0) {
            AC_ERROR << "Seek error.";
            return;
        }

        if (theTimestamp == 0) {
            return;
        }

        // We got to a key frame. Forward until we get to the frame we want.
        while (true) {
            long long myLastDTS = advance();
            if (myLastDTS == UINT_MAX) {
                AC_WARNING << "Seek reached end of file";
                return;
            }

            double myTimestamp = (myLastDTS - _myVideoStream->start_time) / (double)AV_TIME_BASE;
            if (myTimestamp >= theTimestamp) {
                if (myTimestamp >= theTimestamp + myFrameTime) {
                    AC_ERROR << "Seek went into the future. Last dts: " << myTimestamp << endl;
                }
                return;
            }
        }
    }

    long long
    DecoderContext::advance() const {
        int64_t   myLastDTS;
        AVPacket  myPacket;        
        int myFoundPictureFlag = 0;

        while (myFoundPictureFlag == 0) {
            if (av_read_frame(_myFormatContext, &myPacket) < 0) {
                break;
            }
            myLastDTS = myPacket.dts;
    
            // Make sure that the packet is from the actual video stream.
            if (myPacket.stream_index == _myVideoStreamIndex) {
                avcodec_decode_video(&_myVideoStream->codec,
                    _myAdvanceFrame, &myFoundPictureFlag,
                    myPacket.data, myPacket.size);
            }
            av_free_packet(&myPacket);
        }

        // From ffmpeg apiexample.c: some codecs, such as MPEG, transmit the
        // I and P frame with a latency of one frame. You must do the
        // following to have a chance to get the last frame of the video.
        if (!myFoundPictureFlag) {
            avcodec_decode_video(&_myVideoStream->codec,
                                 _myAdvanceFrame, &myFoundPictureFlag,
                                 0, 0);
            myLastDTS += AV_TIME_BASE * _myVideoStream->r_frame_rate_base / _myVideoStream->r_frame_rate;
        }
        if (!myFoundPictureFlag) {
            myLastDTS = UINT_MAX;
        }        
        return myLastDTS;
    }

    double
    DecoderContext::getFrameRate() {
        double myFPS = _myVideoStream->codec.frame_rate / (double) _myVideoStream->codec.frame_rate_base;
        if (myFPS > 1000.0f) {
            myFPS /= 1000.0f;
        }

        return myFPS;
    }
}