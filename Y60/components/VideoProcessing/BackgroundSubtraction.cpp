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

#include "BackgroundSubtraction.h"

using namespace asl;
using namespace std;
using namespace dom;

namespace y60 {

    BackgroundSubtraction::BackgroundSubtraction(const std::string & theName) :
        Algorithm(theName),
        _myResultNode("result"),
        _mySourceRaster(0)
    {
        // _myResultNode.appendChild(Element("red"));
        // _myResultNode.childNode("red")->appendChild(Text(""));
        // _myResultNode.appendChild(Element("green"));
        // _myResultNode.childNode("green")->appendChild(Text(""));
        // _myResultNode.appendChild(Element("blue"));
        // _myResultNode.childNode("blue")->appendChild(Text(""));
    }
  

    void 
    BackgroundSubtraction::configure(const dom::Node & theNode) {
        
        for( unsigned int i=0; i<theNode.childNodesLength(); i++) {
            const std::string myName = theNode.childNode("property",i)->getAttribute("name")->nodeValue();
            const std::string myID = theNode.childNode("property",i)->getAttribute("value")->nodeValue();
            dom::NodePtr myImage = _myScene->getSceneDom()->getElementById(myID);
            AC_PRINT << "configure " << myName;
            if( myImage ) {
                if( myName == "sourceimage") {
                    _mySourceRaster = myImage->getFacade<y60::Image>()->getRasterValue();
                } else if( myName == "backgroundimage") {
                    _myBackgroundRaster = myImage->getFacade<y60::Image>()->getRasterValue();
                    AC_PRINT << "raster " << _myBackgroundRaster;
                } else if( myName == "targetimage") {
                    _myTargetRaster =  myImage->getFacade<y60::Image>()->getRasterValue();
                    _myTargetImage = myImage->getFacade<y60::Image>();
                }
            }
        }   

        // TODO:  add colorspace and dimension checking! [sh]
    }

	void 
    BackgroundSubtraction::onFrame(double t) {
        
        const BGRRaster * mySourceFrame     = dom::dynamic_cast_Value<BGRRaster>(&*_mySourceRaster);
        const BGRRaster * myBackgroundFrame = dom::dynamic_cast_Value<BGRRaster>(&*_myBackgroundRaster);
        const BGRRaster * myTargetFrame     = dom::dynamic_cast_Value<BGRRaster>(&*_myTargetRaster);
        
        BGRRaster::const_iterator itBg   = myBackgroundFrame->begin();
        BGRRaster::iterator itTrgt = const_cast<BGRRaster::iterator>(myTargetFrame->begin());
            
        AC_PRINT <<  myBackgroundFrame;  

        for (BGRRaster::const_iterator itSrc = mySourceFrame->begin(); itSrc != mySourceFrame->end(); ++itSrc,++itBg, ++itTrgt) {
            // redHistogram[static_cast<unsigned int>((*it)[0])]++;
            (*itTrgt)[0] = clampedSub((*itBg)[0], (*itSrc)[0]);  
            (*itTrgt)[1] = clampedSub((*itBg)[1], (*itSrc)[1]);  
            (*itTrgt)[2] = clampedSub((*itBg)[2], (*itSrc)[2]);  
        }
        
        _myTargetImage->triggerUpload();
	}

    unsigned int
    BackgroundSubtraction::clampedSub(unsigned int theFirstValue, unsigned int theSecondValue) {
        return asl::maximum<unsigned int>(theFirstValue, theSecondValue) -  asl::minimum<unsigned int>(theFirstValue, theSecondValue);
    }
}
