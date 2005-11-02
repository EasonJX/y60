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

#ifndef _ac_y60_DecoderContext_h_
#define _ac_y60_DecoderContext_h_

#include "FFMpegPixelEncoding.h"

#include <asl/Ptr.h>
#include <string>
#include <map>

namespace y60 {
   
    class AudioFrame;
    
    class DecoderContext {
        public:
            DecoderContext(const std::string & theFilename);
            ~DecoderContext();

            bool decodeVideo(AVFrame * theVideoFrame);
            bool decodeAudio(AudioFrame * theAudioFrame);
            void seekToTime(double theTime);

            AVStream * getVideoStream() {
                return _myVideoStream;
            }

            AVStream * getAudioStream() {
                return _myAudioStream;
            }

            const std::string & getFilename() const {
                return _myFilename;
            }

            double getEndOfFileTimestamp() const {
                return _myEndOfFileTimestamp;
            }
            void setEndOfFileTimestamp(double theTime) {
                _myEndOfFileTimestamp = theTime;
            }

            unsigned getWidth(); 
            unsigned getHeight();

            PixelFormat getPixelFormat();             
            double getFrameRate();

            unsigned getNumAudioChannels() const;
            void disableAudio();
            
        private:
            long long advance() const;

            AVFormatContext * _myFormatContext;           
            AVStream *        _myVideoStream;
            AVStream *        _myAudioStream;
            AVFrame *         _myAdvanceFrame;
            int               _myVideoStreamIndex;
            int               _myAudioStreamIndex;
            std::string       _myFilename;
            double            _myEndOfFileTimestamp;

            // Demuxing and packet-level buffering support.
            AVPacket* getPacket(bool theGetVideo);
            void clearPacketCache();
            
            typedef std::list<AVPacket *> PacketList;
            PacketList _myAudioPackets;
            PacketList _myVideoPackets;

            AVPacket * _myCurAudioPacket;
            int _myCurPosInAudioPacket;

            bool _myAudioEnabled;
    };

    typedef asl::Ptr<DecoderContext> DecoderContextPtr;
}

#endif

