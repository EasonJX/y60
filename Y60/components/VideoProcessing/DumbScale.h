//============================================================================
//
// Copyright (C) 2007, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#ifndef _AC_DUMPSCALE_H_
#define _AC_DUMPSCALE_H_

#include "Algorithm.h"

namespace y60 {

	class DumbScale : public Algorithm {
		public:
            DumbScale(const std::string & theName);

			static std::string getName() { return "dumpscale"; }
		    void onFrame(double t);
            
            void configure(const dom::Node & theNode);
	        const dom::Node & result() const { 
		        return _myResultNode;
	        }

		private:
            dom::Element _myResultNode;
            dom::ValuePtr _mySourceRaster;
            dom::ValuePtr _myTargetRaster;
            
            y60::ImagePtr _myTargetImage;
            
            unsigned int _myWidth;
            unsigned int _myHeight;
    };
}

#endif
