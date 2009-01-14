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
//   $RCSfile: MovieDecoderBase.h,v $
//   $Author: valentin $
//   $Revision: 1.6 $
//   $Date: 2005/04/19 12:38:29 $
//
//
//=============================================================================

#ifndef _ac_y60_MovieDecoderBase_h_
#define _ac_y60_MovieDecoderBase_h_

#include "y60_video_settings.h"

#include <y60/image/PixelEncoding.h>
#include <y60/base/NodeValueNames.h>
#include <y60/base/IDecoder.h>

#include <asl/base/Ptr.h>
#include <asl/math/Matrix4.h>
#include <asl/base/Stream.h>
#include <string>
#include <asl/base/Exception.h>

namespace asl {
    class ReadableBlock;
}

namespace y60 {
    class Movie;
    /**
     * @ingroup Y60video
     * Abstract base class for Movie Decoders.
     *
     */
    class Y60_VIDEO_EXPORT MovieDecoderBase : public IDecoder {
        public:
            MovieDecoderBase();

            virtual ~MovieDecoderBase();

            void initialize(Movie * theMovie);
/*
            virtual std::string canDecode(const std::string & theUrl, 
                    asl::Ptr<asl::ReadableStreamHandle> theStream = asl::Ptr<asl::ReadableStreamHandle>(0));
*/
            /**
             *
             * @param theSystemTime used for calculation of delta t in free running mode.
             *                      Should be increasing constantly over time. May be ignored
             *                      if the audioclock or the framecache are used.
             * @return current movie time
             */
            virtual double getMovieTime(double theSystemTime); 

            /**
             * Reads a frame from the decoder.
             * @param theTime time of the frame to fetch
             * @param theFrame index of the frame to fetch
             * @param theTargetRaster raster to read the frame into
             * @return real time of the frame return in theTargetRaster
             */
            virtual double readFrame(double theTime, unsigned theFrame, 
                    dom::ResizeableRasterPtr theTargetRaster) = 0;

            /**
             * loads a movie from the file given by theFilename
             * @param theFilename file to load into the decoder
             */
            virtual void load(const std::string & theFilename) = 0;

            /**
             * loads a movie from the stream given by theSource
             * @param theFilename file to identify theSource
             */
            virtual void load(asl::Ptr<asl::ReadableStream> theSource, 
                    const std::string & theFilename);

            int getFrameCount() const; 
            unsigned getFrameWidth() const; 
            unsigned getFrameHeight() const; 
            double getFrameRate() const;
            bool getAudioFlag() const; 
            const asl::Matrix4f & getImageMatrix() const; 
            MoviePlayMode getPlayMode() const; 
            bool getEOF() const; 
            void setEOF(bool theEndOfFileFlag); 
            void setDecodeAudioFlag(bool theDecodeAudioFlag);
            
            /**
             * Starts the movie playback at theStartTime.
             * @param theStartTime start time of the movie. Defaults to 0.0
             */
            virtual void startMovie(double theStartTime = 0.0, bool theStartAudioFlag = true);

            /**
             * Is called before start movie, if a movie was paused before.
             */
            virtual void resumeMovie(double theStartTime = 0.0, bool theResumeAudioFlag = true); 

            /**
             * Stops the movie playback. Future calls of startMovie will start from the 
             * beginning of the movie.
             */
            virtual void stopMovie(bool theStopAudioFlag = true);
            
            /**
             * Pauses the current movie. Playback will continue at the current position after startMovie
             * is called.
             */
            virtual void pauseMovie(bool thePauseAudioFlag = true);
            
            /**
             * Closes the current movie and releases all internal data.
             */
            virtual void closeMovie();

            /**
             * @return new instance of the concrete MovieDecoder.
             */
            virtual asl::Ptr<MovieDecoderBase> instance() const;
            
            virtual const char * getName() const { return "";}

        protected:
            const Movie * getMovie() const;
            Movie * getMovie(); 
            bool getDecodeAudioFlag() const;

        private:
            Movie *        _myMovie;
            bool           _myEndOfFileFlag;
            double         _myMovieTime;
            double         _myLastSystemTime;
            bool           _myDecodeAudioFlag;
    };

    typedef asl::Ptr<MovieDecoderBase> MovieDecoderBasePtr;
}
#endif

