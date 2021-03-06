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

#ifndef _IAudioDecoderFactory_H_
#define _IAudioDecoderFactory_H_

#include "y60_sound_settings.h"

#include "IAudioDecoder.h"

#include <asl/audio/ISampleSink.h>

#include <string>

namespace y60 {

DEFINE_EXCEPTION(DecoderException, asl::Exception);
DEFINE_EXCEPTION(FileNotFoundException, asl::Exception);

class Y60_SOUND_DECL IAudioDecoderFactory
{
    public:
        // Throws a decoder exception if not successful.
        virtual ~IAudioDecoderFactory(){};
        virtual IAudioDecoder* tryCreateDecoder(const std::string& myURI) = 0;
        virtual int getPriority() const = 0; // Decoders with lower priority get checked first.

    private:
        friend bool lessFactory(const IAudioDecoderFactory* a, const IAudioDecoderFactory* b);
};

} // namespace

#endif
