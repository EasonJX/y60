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
//   $RCSfile: DecoderManager.h,v $
//   $Author: pavel $
//   $Revision: 1.2 $
//   $Date: 2005/04/24 00:41:17 $
//
//
//=============================================================================

#ifndef _y60_decoder_manager_included_
#define _y60_decoder_manager_included_

#include "IDecoder.h"

#include <asl/base/Logger.h>
#include <asl/base/Singleton.h>

#define DB(x) //x

namespace y60 {

    /**
      DecoderManager holds decoders of all types and finds the (hopefully) correct one for a given ReadableStream.
      <ol>
      <li>Register your decoder by calling addDecoder()
      <li>Find a decoder <i>of a specific type</i> by calling findDecoder<DECODERTYPE>()
      <br>
          e.g. Scene Decoders:
      <br>
               y60::StlImport & y60::X3dImport are specific decoders derived from y60::ISceneDecoder.
               We use findDecoder<ISceneDecoder>() in Scene::load() to decode a scene accordingly.

      </ol>
      */
    DEFINE_EXCEPTION(DecoderManagerException, asl::Exception);

class DecoderManager : public asl::Singleton<DecoderManager> {
    public:
        DecoderManager() {};
        virtual ~DecoderManager() {};
        void addDecoder(IDecoderPtr theDecoder);
        /**
        * Trys to find a decoder, that accepts the url/stream.
        * @param theUrl can be used to check for known file extensions.
        * @param theStream should also be provided if possible, to check for magic numbers, etc.
        * @return The first decoder, that accapts the given url/stream.
        */
        template <class DECODERTYPE>
        asl::Ptr<DECODERTYPE> findDecoder(const std::string & theUrl, asl::Ptr<asl::ReadableStreamHandle> theSource = asl::Ptr<asl::ReadableStreamHandle>(0)) {
            AC_TRACE << "checking decoders for url '"<< theUrl <<"', number of decoders = " << _myDecoders.size();
            for (std::vector<IDecoderPtr>::size_type i = 0; i < _myDecoders.size(); ++i) {
                AC_TRACE << "checking decoder #" << i;
                asl::Ptr<DECODERTYPE> myDecoder = dynamic_cast_Ptr<DECODERTYPE>(_myDecoders[i]);
                if (myDecoder) {
                    std::string myMimeType = _myDecoders[i]->canDecode(theUrl, theSource);
                    AC_TRACE << "decoder returned '" << myMimeType << "'";
                    if (!myMimeType.empty()) {
                        return myDecoder;
                    }
                }
            }
            return asl::Ptr<DECODERTYPE>(0);
        }

        /**
        * Collects all decoders that accept the url/stream.
        * @param theUrl can be used to check for known file extensions.
        * @param theStream should also be provided if possible, to check for magic numbers, etc.
        * @return all decoders that accept the given url/stream.
        */
        template <class DECODERTYPE>
        std::vector<asl::Ptr<DECODERTYPE> > findAllDecoders(const std::string & theUrl, asl::Ptr<asl::ReadableStreamHandle> theSource = asl::Ptr<asl::ReadableStreamHandle>(0)) {
            std::vector<asl::Ptr<DECODERTYPE> > myCapableDecoders;
            for (std::vector<IDecoderPtr>::size_type i = 0; i < _myDecoders.size(); ++i) {
                AC_TRACE << "checking decoder #" << i;
                asl::Ptr<DECODERTYPE> myDecoder = dynamic_cast_Ptr<DECODERTYPE>(_myDecoders[i]);
                if (myDecoder) {
                    std::string myMimeType = myDecoder->canDecode(theUrl, theSource);
                    AC_TRACE << "decoder returned '" << myMimeType << "'";
                    if (!myMimeType.empty()) {
                        myCapableDecoders.push_back(myDecoder);
                    }
                }
            }
            return myCapableDecoders;
        }
    private:
        std::vector<IDecoderPtr> _myDecoders;
};


}

#undef DB

#endif
