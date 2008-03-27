//=============================================================================
// Copyright (C) 2004-2006, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#include "FFMpegDecoder1.h"

#include <y60/Movie.h>

#include <asl/Logger.h>
#include <asl/file_functions.h>
#include <asl/string_functions.h>

#include <iostream>

#define DB(x) // x

using namespace std;
using namespace asl;

extern "C"
EXPORT asl::PlugInBase * y60FFMpegDecoder1_instantiatePlugIn(asl::DLHandle myDLHandle) {
    return new y60::FFMpegDecoder1(myDLHandle);
}

namespace y60 {

    FFMpegDecoder1::VideoFrame::VideoFrame(unsigned theBufferSize) {
        _myBuffer = new unsigned char[theBufferSize];
        DB(AC_DEBUG << "VideoFrame buffer=" << (void*)_myBuffer);
    }

    FFMpegDecoder1::VideoFrame::~VideoFrame() {
        DB(AC_DEBUG << "~VideoFrame buffer=" << (void*)_myBuffer);
        delete[] _myBuffer;
        _myBuffer = 0;
    }

    // FFMpegDecoder1
    FFMpegDecoder1::FFMpegDecoder1(asl::DLHandle theDLHandle) :
        PlugInBase(theDLHandle),
        _myFormatContext(0), _myFrame(0),
        _myVStream(0),
        _myLastVStreamIndex(-1),
        _myStartTimestamp(0),
        _myLastVideoTimestamp(0),
        _myDestinationPixelFormat(PIX_FMT_BGR24),
        _myBytesPerPixel(0)
    {
        DB(AC_DEBUG << "instantiated an FFMpegDecoder1...");
    }

    FFMpegDecoder1::~FFMpegDecoder1() {
        closeMovie();
        DB(AC_DEBUG << "destructing an FFMpegDecoder1...");
    }

    asl::Ptr<MovieDecoderBase> FFMpegDecoder1::instance() const {
        return asl::Ptr<MovieDecoderBase>(new FFMpegDecoder1(getDLHandle()));
    }

    std::string
    FFMpegDecoder1::canDecode(const std::string & theUrl, asl::ReadableStream * theStream) {
        string myFilename = asl::toLowerCase(asl::getFilenamePart(theUrl));
        if (asl::toLowerCase(asl::getExtension(theUrl)) == "mpg" ||
            asl::toLowerCase(asl::getExtension(theUrl)) == "m2v" ||
            myFilename == "dvr0") {
            AC_INFO << "FFMpegDecoder1 can decode :" << theUrl << endl;
            return MIME_TYPE_MPG;
        } else {
            AC_INFO << "FFMpegDecoder1 can not decode :" << theUrl << " responsible for extensions mpg and m2v" << endl;
            return "";
        }
    }

    void
    FFMpegDecoder1::closeMovie() {
        AC_DEBUG << "FFMpegDecoder1::closeMovie";
        if (_myVStream) {
            avcodec_close(_myVStream->codec);
            _myLastVStreamIndex = -1;
            _myVStream = 0;
        }

        if (_myFrame) {
            av_free(_myFrame);
            _myFrame = 0;
        }

        av_close_input_file(_myFormatContext);
        _myFormatContext = 0;
    }

    void
    FFMpegDecoder1::setMovieParameters(int myIndex) {
        AVStream * myVStream = _myFormatContext->streams[myIndex];

        if (myVStream == 0) {
            return;
        }

        Movie * myMovie = getMovie();

        int myWidth, myHeight;
        myWidth = myVStream->codec->width;
        myHeight = myVStream->codec->height;
        myMovie->set<ImageWidthTag>(myWidth);
        myMovie->set<ImageHeightTag>(myHeight);
    }

    void
    FFMpegDecoder1::load(const std::string & theFilename) {
        asl::Time myLoadStartTime;

        // register all formats and codecs
        static bool avRegistered = false;
        if (!avRegistered) {
            AC_INFO << "FFMpegDecoder1 avcodec version " << LIBAVCODEC_IDENT;
            av_log_set_level(AV_LOG_ERROR);
            av_register_all();
            avRegistered = true;
        }

        // open file
        if (av_open_input_file(&_myFormatContext, theFilename.c_str(), 0, 0, 0) < 0) {
            throw FFMpegDecoderException(std::string("Unable to open input file: ") + theFilename, PLUS_FILE_LINE);
        }

        av_read_play(_myFormatContext);
        if (av_find_stream_info(_myFormatContext) < 0) {
            throw FFMpegDecoderException(std::string("Unable to find stream info: ") + theFilename, PLUS_FILE_LINE);
        }

        // find video/audio streams
        int myIndex = 0;
        for (unsigned i = 0; i < _myFormatContext->nb_streams; ++i) {
            int myCodecType;
            myCodecType = _myFormatContext->streams[i]->codec->codec_type;
            // if (_myVStreamIndex == -1 && myCodecType == CODEC_TYPE_VIDEO) {
            if (myCodecType == CODEC_TYPE_VIDEO) {
                myIndex = i;
                _myVStream = _myFormatContext->streams[i];
                break;
            }
        }

        if (_myVStream == 0) {
            throw FFMpegDecoderException(std::string("No video stream found: ") + theFilename, PLUS_FILE_LINE);
        }

        // open codec
        AVCodec * myCodec = avcodec_find_decoder(_myVStream->codec->codec_id);
        if (!myCodec) {
            throw FFMpegDecoderException(std::string("Unable to find decoder: ") + theFilename, PLUS_FILE_LINE);
        }

        if (avcodec_open(_myVStream->codec, myCodec) < 0 ) {
            throw FFMpegDecoderException(std::string("Unable to open codec: ") + theFilename, PLUS_FILE_LINE);
        }

        // allocate frame for YUV data
        _myFrame = avcodec_alloc_frame();

        int myWidth = _myVStream->codec->width;
        int myHeight = _myVStream->codec->height;


        // pixelformat stuff
        Movie * myMovie = getMovie();
		TextureInternalFormat myTextureFormat = TextureInternalFormat(getEnumFromString(myMovie->get<RasterPixelFormatTag>(), TextureInternalFormatStrings));

        TextureInternalFormat myTargetPixelFormat = myTextureFormat;
        if (myMovie->get<TargetPixelFormatTag>() != "") {
        	myTargetPixelFormat = TextureInternalFormat(getEnumFromString(myMovie->get<TargetPixelFormatTag>(), TextureInternalFormatStrings));
        }

        switch (myTargetPixelFormat) {
            case TEXTURE_IFMT_RGBA8:
                AC_TRACE << "Using TEXTURE_IFMT_RGBA8 pixels";
                _myDestinationPixelFormat = PIX_FMT_BGRA;
                _myBytesPerPixel = 4;
                myMovie->createRaster(myWidth, myHeight, 1, y60::RGBA);
                myMovie->addRasterValue(createRasterValue( y60::RGBA, myWidth, myHeight), y60::RGBA, 1);                
                break;
            case TEXTURE_IFMT_ALPHA:
                AC_TRACE << "Using GRAY pixels";
                _myDestinationPixelFormat = PIX_FMT_GRAY8;
                _myBytesPerPixel = 1;
                myMovie->createRaster(myWidth, myHeight, 1, y60::ALPHA);
                myMovie->addRasterValue(createRasterValue( y60::ALPHA, myWidth, myHeight), y60::ALPHA, 1);                
                break;
            case TEXTURE_IFMT_LUMINANCE8:
                AC_TRACE << "Using GRAY pixels";
                _myDestinationPixelFormat = PIX_FMT_GRAY8;
                _myBytesPerPixel = 1;
                myMovie->createRaster(myWidth, myHeight, 1, y60::GRAY);
                myMovie->addRasterValue(createRasterValue( y60::GRAY, myWidth, myHeight), y60::GRAY, 1);                
                break;
            case TEXTURE_IFMT_RGB8:
            default:
                AC_TRACE << "Using BGR pixels";
                _myDestinationPixelFormat = PIX_FMT_BGR24;
                _myBytesPerPixel = 3;
                myMovie->createRaster(myWidth, myHeight, 1, y60::BGR);
                myMovie->addRasterValue(createRasterValue( y60::BGR, myWidth, myHeight), y60::BGR, 1);                
                break;
        }

        myMovie->getRasterPtr(Movie::PRIMARY_BUFFER)->clear();
		myMovie->getRasterPtr(Movie::SECONDARY_BUFFER)->clear();

        // framerate
        double myFPS;
        myFPS = (1.0 / av_q2d(_myVStream->codec->time_base));
        myMovie->set<FrameRateTag>(myFPS);

        // duration
        AVCodecContext * myVCodec = _myVStream->codec;
        if (_myVStream->duration == AV_NOPTS_VALUE || _myVStream->duration <= 0 ||
                myVCodec->codec_id == CODEC_ID_MPEG1VIDEO || 
                myVCodec->codec_id == CODEC_ID_MPEG2VIDEO ) {
            myMovie->set<FrameCountTag>(-1);
        } else {
            myMovie->set<FrameCountTag>(int(myFPS * _myVStream->duration
                                            * av_q2d(_myVStream->time_base)));
        }


        _myLastVideoTimestamp = 0;

        // Get Starttime
        if (_myVStream->start_time != AV_NOPTS_VALUE) {
            _myStartTimestamp = _myVStream->start_time;
        } else {
            _myStartTimestamp = 0;
        }
        AC_DEBUG << "startTimestamp=" << _myStartTimestamp;

        asl::Time myLoadEndTime;
        AC_INFO << "Load time " << (myLoadEndTime - myLoadStartTime) << "s";
    }

    double
    FFMpegDecoder1::readFrame(double theTime, unsigned theFrame, dom::ResizeableRasterPtr theTargetRaster) {
        // theTime is in seconds,
        int64_t myTimeUnitsPerSecond = (int64_t)(1/ av_q2d(_myVStream->time_base));
        int64_t myFrameTimestamp = (int64_t)(theTime * myTimeUnitsPerSecond) + _myStartTimestamp;

        AC_DEBUG << "time=" << theTime << " timestamp=" << myFrameTimestamp <<
                " myTimeUnitsPerSecond=" << myTimeUnitsPerSecond;
        AC_DEBUG << "_myLastVideoTimestamp=" << _myLastVideoTimestamp;

        if (!decodeFrame(myFrameTimestamp, theTargetRaster)) {
            AC_DEBUG << "EOF reached for timestamp=" << myFrameTimestamp;
            setEOF(true);
        } else {
            DB(AC_TRACE << "FFMpegDecoder1::readFrame decoded timestamp=" << myFrameTimestamp);
        }

        return theTime;
    }

    // TODO: Make theTimestamp an input-only parameter.
    bool
    FFMpegDecoder1::decodeFrame(int64_t & theTimestamp, dom::ResizeableRasterPtr theTargetRaster) {
        AC_DEBUG << "--- decodeFrame ---" << endl;
        int64_t myTimeUnitsPerSecond = (int64_t)(1/ av_q2d(_myVStream->time_base));
        int64_t myTimePerFrame = (int64_t)(myTimeUnitsPerSecond / getFrameRate());


        if (_myVStream->index_entries != NULL) {
            if (_myLastVideoTimestamp != AV_NOPTS_VALUE &&
                (theTimestamp < _myLastVideoTimestamp || theTimestamp >
                 _myLastVideoTimestamp + (2*myTimePerFrame)))
            {
//            cout <<"seek, theTimeStanp: " << theTimestamp << " last timestamp : "
//                  << _myLastVideoTimestamp << " Frametime : " << myTimePerFrame <<endl;

                // [ch] If we seek directly to the seek timestamp the packet sometimes cannot be decoded afterwards.
                // Therefore this workaround seeks a bit (0.01 Frames) before the requested timestamp
                int64_t mySeekTimestamp = theTimestamp-myTimePerFrame/100;
                if (mySeekTimestamp < 0) {
                    mySeekTimestamp = 0;
                }
//            int64_t mySeekTimestamp = theTimestamp;
                AC_DEBUG << "SEEK timestamp=" << theTimestamp << " lastVideoTimestamp="
                         << _myLastVideoTimestamp << " seek=" << mySeekTimestamp;

                int myResult = av_seek_frame(_myFormatContext, _myLastVStreamIndex, mySeekTimestamp,
                                             AVSEEK_FLAG_BACKWARD);
                if (myResult < 0) {
                    AC_ERROR << "Could not seek to timestamp " << mySeekTimestamp;
                }
            }
        }

        VideoFramePtr myVideoFrame(0);

        AVPacket myPacket;
        memset(&myPacket, 0, sizeof(myPacket));

        // until a frame is found or eof
        while (true) {
            // EOF handling
            int myError;
            if ((myError = av_read_frame(_myFormatContext, &myPacket)) < 0) {
                av_free_packet(&myPacket);
#if 1 
                /*
                 * Some codecs, such as MPEG, transmit the I and P frame with a
                 * latency of one frame. You must do the following to have a
                 * chance to get the last frame of the video
                 */
                int frameFinished = 0;
                int myLen = avcodec_decode_video(_myVStream->codec, _myFrame, &frameFinished, NULL, 0);
                if (frameFinished) {
                    _myLastVideoTimestamp += myTimePerFrame;

                    // last frame found
                    if (_myLastVideoTimestamp >= theTimestamp) {
                        break;
                    } else {
                        continue;
                    }
                }
#endif
                AC_DEBUG << "eof reached : error: " << myError << " " << (myError==-11);

                return false;
            }

            //       if (myPacket.stream_index == _myVStreamIndex) {
            int myCodecType;
            myCodecType = _myFormatContext->streams[myPacket.stream_index]->codec->codec_type;
            if (myCodecType == CODEC_TYPE_VIDEO) {
                if (_myLastVStreamIndex != myPacket.stream_index) {
                    setMovieParameters(myPacket.stream_index);
                }

                int frameFinished = 0;
                unsigned char* myData = myPacket.data;
                int myDataLen = myPacket.size;

                while (frameFinished == 0 && myDataLen > 0) {
                    int myLen = avcodec_decode_video(_myVStream->codec,
                                                     _myFrame, &frameFinished,
                                                     myData, myDataLen);
                    if (myLen < 0) {
                        AC_ERROR << "av_decode_video error";
                        break;
                    }
                    myData += myLen;
                    myDataLen -= myLen;
                }

                AC_DEBUG << "decoded dts=" << myPacket.dts << " pts=" << myPacket.pts
                        << " finished=" << frameFinished;
                if (frameFinished == 0) {
                    continue;
                }
                _myLastVideoTimestamp = myPacket.dts;

                // suitable framestamp?
                if (_myLastVideoTimestamp >= theTimestamp) {
                    DB(AC_TRACE << "found " << _myLastVideoTimestamp << " for " << theTimestamp);
                    break;
                }
            }
            av_free_packet(&myPacket);
        }

        convertFrame(_myFrame, theTargetRaster);
        // store actual timestamp

        theTimestamp = myPacket.dts;
        AC_DEBUG << "Packet dts: " << myPacket.dts << ", pts: " << myPacket.pts;

        _myLastVStreamIndex = myPacket.stream_index;
        av_free_packet(&myPacket);

        return true;
    }

    void FFMpegDecoder1::convertFrame(AVFrame* theFrame, dom::ResizeableRasterPtr theTargetRaster) {
        theTargetRaster->resize(getFrameWidth(), getFrameHeight());
        if (!theFrame) {
            AC_ERROR << "FFMpegDecoder1::decodeVideoFrame invalid AVFrame";
            return;
        }
        unsigned int myLineSizeBytes = getFrameWidth() * _myBytesPerPixel;

        AVPicture myDestPict;
        myDestPict.data[0] = theTargetRaster->pixels().begin();
        myDestPict.data[1] = theTargetRaster->pixels().begin()+1;
        myDestPict.data[2] = theTargetRaster->pixels().begin()+2;

        myDestPict.linesize[0] = myLineSizeBytes;
        myDestPict.linesize[1] = myLineSizeBytes;
        myDestPict.linesize[2] = myLineSizeBytes;


        AVCodecContext * myVCodec = _myVStream->codec;
        img_convert(&myDestPict, _myDestinationPixelFormat, (AVPicture*)theFrame,
                    myVCodec->pix_fmt, myVCodec->width, myVCodec->height);
    }
}
