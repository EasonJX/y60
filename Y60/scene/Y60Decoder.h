//============================================================================
//
// Copyright (C) 2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================
//
//   $RCSfile: $
//
//   $Author: $
//
//   $Revision: $
//
//=============================================================================

#ifndef _ac_scene_Y60Decoder_h_
#define _ac_scene_Y60Decoder_h_

#include "ISceneDecoder.h"
#include <asl/Exception.h>

namespace y60 {

    DEFINE_EXCEPTION(Y60DecodeException, asl::Exception);

    const std::string MIME_TYPE_X60 = "model/x60";
    const std::string MIME_TYPE_B60 = "model/b60";
/**
 * @ingroup Y60scene
 * Decoder for y60 files (x60, b60)
 */ 
class Y60Decoder : public ISceneDecoder {
    public:
        virtual bool decodeScene(asl::ReadableStream &, dom::DocumentPtr theScene);
        /**
         * checks if the given file can be decoded
         * @param theUrl URL to read from
         * @param theStream stream to read from
         * @todo implement support for streamed b60
         * @retval MIME_TYPE_X60 This file is an xml x60 file and can be decoded
         * @retval MIME_TYPE_B60 This file is an binary b60 file and can be decoded
         * @return "" This file cannot be decoded.
         */
        virtual std::string canDecode(const std::string & theUrl, asl::ReadableStream * theStream = 0);
        virtual bool setProgressNotifier(IProgressNotifierPtr theNotifier);
    private:
        void loadXmlFile(asl::ReadableStream & theXmlSource, bool theBinaryFlag);
};

typedef asl::Ptr<Y60Decoder> Y60DecoderPtr;
}

#endif // _ac_scene_Y60Decoder_h_

