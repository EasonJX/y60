//============================================================================
// Copyright (C) 2000-2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================
//
//   $Id: RankedFeature.cpp,v 1.1 2005/03/24 23:35:56 christian Exp $
//   $RCSfile: RankedFeature.cpp,v $
//   $Author: christian $
//   $Revision: 1.1 $
//   $Date: 2005/03/24 23:35:56 $
//
//
//  Description: This file contains all the wellknown names for the
//               SOM
//
// (CVS log at the bottom of this file)
//
//=============================================================================

#include "RankedFeature.h"

#include <asl/Block.h>
#include <dom/Value.h>

#include <string>
#include <vector>

namespace y60 {

    std::ostream &
    operator << (std::ostream & theStream, const RankedFeature & theRankedFeature) {
        theStream << theRankedFeature._myRanking;
        theStream << "[";
        for (unsigned i = 0; i < theRankedFeature._myFeature.size(); ++i) {
            theStream << theRankedFeature._myFeature[i];
            if (i < theRankedFeature._myFeature.size() - 1) {
                theStream << ",";
            }
        }
        theStream << "]";
        return theStream;
    }

    std::istream &
    operator >> (std::istream & theStream, RankedFeature & theRankedFeature) {
            theStream >> theRankedFeature._myRanking;

            if (theStream.fail()) {
                return theStream;
            }

            char myChar;
            theStream >> myChar;

            if (myChar != '[') {
                theStream.setstate(std::ios::failbit);
                return theStream;
            }

            std::string myElement;
            do {
                theStream >> myChar;
                if ((myChar == ',') || (myChar == ']')) {
                    theRankedFeature._myFeature.push_back(myElement);
                    myElement.clear();
                    if (myChar == ']') {
                        break;
                    }
                } else {
                    myElement += myChar;
                }
            } while ( ! theStream.eof());

            if (myChar != ']') {
                theStream.setstate(std::ios::failbit);
            }

            return theStream;
    }
}

namespace dom {
    void binarize(const y60::RankedFeature & myValue, asl::WriteableStream & theDest) {
        theDest.appendFloat32(myValue._myRanking);
        dom::binarize(myValue._myFeature,theDest);
    };

	asl::AC_SIZE_TYPE debinarize(y60::RankedFeature & myValue, const asl::ReadableStream & theSource, asl::AC_SIZE_TYPE thePos) {
        thePos = theSource.readFloat32(myValue._myRanking,thePos);
        return dom::debinarize(myValue._myFeature,theSource,thePos);
    };
}
