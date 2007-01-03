//=============================================================================
//
// Copyright (C) 1993-2006, ART+COM AG
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//
//=============================================================================

#ifndef INCL_GRAINSOURCE
#define INCL_GRAINSOURCE

#include <asl/ISampleSink.h>
#include <asl/SampleSource.h>
#include <asl/AudioBuffer.h>

#include <vector>

namespace y60 {

    class GrainSource;

    typedef asl::Ptr<GrainSource, asl::MultiProcessor, asl::PtrHeapAllocator<asl::MultiProcessor> > GrainSourcePtr;
    typedef asl::WeakPtr<GrainSource, asl::MultiProcessor, asl::PtrHeapAllocator<asl::MultiProcessor> > GrainSourceWeakPtr;
    
    class GrainSource : public asl::SampleSource, public asl::ISampleSink {

    public:

        GrainSource(const std::string& theName, asl::SampleFormat theSampleFormat, unsigned theSampleRate, unsigned theNumChannels);
        virtual ~GrainSource();

        // inherited from SampleSource
        virtual void deliverData(asl::AudioBufferBase& theBuffer);

        // inherited from ISampleSink
        virtual bool queueSamples(asl::AudioBufferPtr& theBuffer); // use it to fill the internal audio data buffer

        void clearAudioData(); 

        void setAudioData(const asl::AudioBufferPtr& theAudioData);

        void setGrainSize(unsigned theSize);
        unsigned getGrainSize() const;

        void setGrainSizeJitter(unsigned theJitter);
        unsigned getGrainSizeJitter() const;

        void setGrainRate(unsigned theRate);
        unsigned getGrainRate() const;

        void setGrainRateJitter(unsigned theJitter);
        unsigned getGrainRateJitter() const;

        void setGrainPosition(float thePosition); 
        float getGrainPosition() const;

        void setGrainPositionJitter(float theJitter);
        float getGrainPositionJitter() const;

        void setTransposition(unsigned theTransposition);
        unsigned getTransposition() const;

        void setTranspositionJitter(unsigned theJitter);
        unsigned getTranspositionJitter() const;

        unsigned jitterValue(unsigned theValue, unsigned theJitter) const;
        float jitterValue(float theValue, float theJitter) const;

    private:

        asl::AudioBufferPtr _myAudioData;        // the buffer containing the audio data to granuralize

        unsigned _myGrainSize;              // length of a grain (in ms)
        unsigned _myGrainSizeJitter;
        unsigned _myGrainRate;              // time interval when the next grain should be played (in ms)
        unsigned _myGrainRateJitter;
        float    _myGrainPosition;          // relative position in the audiobuffer where the grain is taken from 
        float     _myGrainPositionJitter;    
        // XXXX to be implemented yet .... (needs resampling for slower or faster playback)
        int      _myTransposition;          // transposition in cents
        int      _myTranspositionJitter;    
        // panning ??

        // some helper members
        asl::AudioBufferPtr _myOverlapBuffer;    // a little space to save overlapping samples between two deliverData calls
        unsigned _myGrainOffset;            // offset of the first grain in our frame buffer
        unsigned _myLastBuffersize;         // because subsequent framebuffers have different sizes, we need to store the last buffer size

        std::vector<float> _myWindowBuffer; // window function applied to the current grain (i.e. hann window)

        void createOverlapBuffer(unsigned theGrainSize);
        void createWindowBuffer(unsigned theWindowSize);

    };
}


#endif // INCL_GRAINSOURCE
