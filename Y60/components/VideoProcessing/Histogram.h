//============================================================================
//
// Copyright (C) 2000-2001, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================

#ifndef _ac_Histogram_h_
#define _ac_Histogram_h_

#include "Algorithm.h"

namespace y60 {

	class Histogram : public Algorithm {
		public:
            Histogram();

			static std::string getName() { return "histogram"; }
		    void onFrame(dom::ValuePtr theRaster, double t);

            void configure(const dom::Node & theNode);
	        const dom::Node & result() const { 
		        return _myResultNode;
	        }
		private:
            dom::Element _myResultNode;
	};

}

#endif
