//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================

#ifndef _ac_y60_SoundManager_h_
#define _ac_y60_SoundManager_h_

#include "Sound.h"
#include "SoundCacheItem.h"
#include "IAudioDecoderFactory.h"

#include <asl/Singleton.h>
#include <asl/Stream.h>
#include <asl/ThreadLock.h>
#include <asl/PosixThread.h>

#include <string>
#include <vector>

namespace y60 {

    class SoundManager : public asl::Singleton<SoundManager>, private PosixThread {
        friend class asl::SingletonManager;
    public:
        virtual ~SoundManager();

        virtual void setSysConfig(const asl::Time& myLatency, const std::string& myDeviceName = "");
        virtual void setAppConfig(unsigned mySampleRate, unsigned numOutputChannels = 2, 
                bool useDummy = false);

        // Virtual so they can be called from outside the plugin.
        virtual void registerDecoderFactory(IAudioDecoderFactory* theFactory);
        virtual void unregisterDecoderFactory(IAudioDecoderFactory* theFactory);

        virtual SoundPtr createSound(const std::string & theURI);
        virtual SoundPtr createSound(const std::string & theURI, bool theLoop);
        virtual SoundPtr createSound(const std::string & theURI, bool theLoop,
                bool theUseCache);
        //virtual SoundPtr createSound(const std::string & theURI, 
        //        asl::Ptr < asl::ReadableStream > theStream, bool theLoop = false);
        virtual void setVolume(float theVolume);
        virtual void fadeToVolume(float theVolume, float theTime);
        virtual float getVolume() const; 
        virtual unsigned getNumSounds() const;
        virtual void stopAll();
        virtual bool isRunning() const;

        // Caching stuff
        void preloadSound(const std::string& theURI);
        void deleteCacheItem(const std::string& theURI);
        
        virtual void update();
        
    private:
        SoundManager();
        void run();
        IAudioDecoder * createDecoder(const std::string & theURI);

        void addCacheItem(SoundCacheItemPtr theItem);
        SoundCacheItemPtr getCacheItem(const std::string & theURI) const;
        
        asl::ThreadLock _myLock;
        std::vector < SoundWeakPtr > _mySounds;

        // Sorted according to decoder priority.
        std::vector < IAudioDecoderFactory* > _myDecoderFactories; 
        IAudioDecoderFactory* _myFFMpegDecoderFactory;

        // Cached sounds indexed by uri.
        typedef map<std::string, SoundCacheItemPtr> CacheMap;
        CacheMap _myCache;
    };
}

#endif
