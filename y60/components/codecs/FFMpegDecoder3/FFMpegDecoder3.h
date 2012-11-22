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
*/

#ifndef _ac_y60_FFMpegDecoder3_h_
#define _ac_y60_FFMpegDecoder3_h_

#include "y60_ffmpegdecoder3_settings.h"

#if defined(_MSC_VER)
#    pragma warning(push,1)
#endif
extern "C" {
#    include <libavformat/avformat.h>
#    if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(51,38,0)
#        include <libswscale/swscale.h>
#    endif
}
#if defined(_MSC_VER)
#    pragma warning(pop)
#endif

#ifndef AV_VERSION_INT
#define AV_VERSION_INT(a,b,c) (a<<16 | b<<8 | c)
#endif

#include <y60/video/VideoMsgQueue.h>
#include <y60/video/AsyncDecoder.h>
#include <y60/video/MovieEncoding.h>

#include <asl/base/PlugInBase.h>
#include <asl/base/Ptr.h>

#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <string>
#include <list>




namespace AudioBase {
    class BufferedSource;
}

namespace y60 {
    class AsyncDemuxer;
    typedef asl::Ptr<AsyncDemuxer> AsyncDemuxerPtr;
    typedef asl::Ptr<boost::thread> DecodeThreadPtr;

    DEFINE_EXCEPTION(FFMpegDecoder3Exception, asl::Exception);
    const std::string MIME_TYPE_MPG = "application/mpg";

    /**
     * @ingroup Y60video
     * Asyncronous decoder using ffmpeg. It features audio and
     * video decoding
     *
     */
    class FFMpegDecoder3 :
        public AsyncDecoder,
        public asl::PlugInBase
    {
        static const double AUDIO_BUFFER_SIZE;   ///< Audio cache size in seconds.

    public:
        //////////////
        //mainthread//
        //////////////
        FFMpegDecoder3(asl::DLHandle theDLHandle);
        virtual ~FFMpegDecoder3();

        virtual asl::Ptr<MovieDecoderBase> instance() const;
        std::string canDecode(const std::string & theUrl, asl::Ptr<asl::ReadableStreamHandle> theStream = asl::Ptr<asl::ReadableStreamHandle>());

        /**
         * loads a movie from the file given by theFilename
         * @param theFilename file to load into the decoder
         */
        void load(const std::string & theFilename);
        double readFrame(double theTime, unsigned, RasterVector theTargetRaster);

        /**
         * Starts movie decoding
         * @param theStartTime movie-time to start decoding at.
         */
        void startMovie(double theStartTime = 0.0f, bool theStartAudioFlag = true);

        void resumeMovie(double theStartTime = 0.0f, bool theResumeAudioFlag = true);

        void loop();
        /**
         * Called to stop the decoding.
         */
        void stopMovie(bool theStopAudioFlag = true);
        void closeMovie();

        const char * getName() const { return "FFMpegDecoder3"; }

        void shutdown();

    private:
        void openStreams();
        void setupVideo(const std::string & theFilename);
        void setupAudio(const std::string & theFilename);
        void getVideoProperties();
        bool shouldSeek(double theCurrentTime, double theDestTime);
        void seek(double theDestTime);
        void checkAudioStream();


        VideoMsgPtr _myLastVideoFrame;
        AVFormatContext * _myFormatContext;
        unsigned int _myAStreamIndexDom; // this index counts from 0, comes vom y60-dom
        int64_t _myVideoStartTimestamp;
        bool _hasShutDown;
        bool _isStreamingMedia;

        ///////////////
        //all threads//
        ///////////////
        VideoMsgQueue _myMsgQueue;
        DecodeThreadPtr _myVideoDecodeThread;
        DecodeThreadPtr _myAudioDecodeThread;
        //for thread sync
        boost::mutex _myAudioMutex;
        boost::mutex _myVideoMutex;
        boost::mutex _mySeekMutex;
        boost::condition _myAudioCondition;
        boost::condition _myVideoCondition;
        boost::condition _mySeekCondition;
        bool _myAudioIsEOF;
        bool _mySeekRequested;

        //should be locked
        AVStream * _myVStream;
        int _myAStreamIndex;    // this index points in ffmpeg stream ordering
        AVStream * _myAStream;
        AVFrame * _myFrame;
        AsyncDemuxerPtr _myDemux;
        bool _myAdjustAudioOffsetFlag;

        ////////////////////////////
        //decode threads////////////
        ////////////////////////////

        void run_videodecode();
        void run_audiodecode();
        /**
         *  updates the Framecache depending on the current position
         */
        bool decodeFrame();
        bool readAudio();
        /**
         * Add an audio packet to the buffered source.
         * @param thePacket packet to add
         * @retval true if successful
         */
        void addAudioPacket(const AVPacket & thePacket);
        /**
         * Add theFrame to the framecache with the timestamp theTimestamp.
         * @throws asl::ThreadSemaphore::ClosedException
         * @param theFrame frame to put into the cache
         * @param theTimestamp timestamp to use
         */
        void addCacheFrame(AVFrame* theFrame, double theTime);
        void convertFrame(AVFrame* theFrame, unsigned char* theBuffer);
        VideoMsgPtr createFrame(double theTimestamp);

        template<typename T>
        int downmix5p1ToStereo(T * theBuffer, int theBytesDecoded) {
            T * myStereoPtr = theBuffer;
            T * my5p1Ptr = theBuffer;
            for (int i = 0; i < theBytesDecoded/3; ++i) {
                *myStereoPtr++ = *my5p1Ptr;
                *myStereoPtr++ = *(my5p1Ptr+2);
                my5p1Ptr += 6;
            }
            return theBytesDecoded/3;
        }
        
    


        ReSampleContext * _myResampleContext;
        static asl::Block _myResampledSamples;

        // only initialized in main thread, then read from in both 
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(52,20,0)
        PixelFormat
#else
        int
#endif
        _myDestinationPixelFormat;
        double _myVideoStreamTimeBase;
        int _myVStreamIndex;
        std::vector<int> _myAllAudioStreamIndicies;
        double _myLastFrameTime;  // Only used for mpeg1/2 end of file handling.

        // worker thread values to prevent dom access and thus race conditions.
        double _myFrameRate;
        unsigned _myMaxCacheSize;
        int _myFrameWidth;
        int _myFrameHeight;
        unsigned  _myBytesPerPixel;

    };
    typedef asl::Ptr<FFMpegDecoder3> FFMpegDecoder3Ptr;
}

#endif
