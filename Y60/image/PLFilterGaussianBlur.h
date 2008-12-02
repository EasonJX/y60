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
//   $Id: PLFilterGaussianBlur.h,v 1.1 2005/03/24 23:36:00 christian Exp $
//   $RCSfile: PLFilterGaussianBlur.h,v $
//   $Author: christian $
//   $Revision: 1.1 $
//   $Date: 2005/03/24 23:36:00 $
//
//
//  Description:
//
//
//=============================================================================

#ifndef _ac_y60_PLFilterGaussianBlur_h_
#define _ac_y60_PLFilterGaussianBlur_h_

#include <vector>
#include <asl/base/Logger.h>

#if defined(_MSC_VER)
#   pragma warning (push,1)
#endif //defined(_MSC_VER)
#include <paintlib/plpaintlibdefs.h>
#include <paintlib/Filter/plfilter.h>
#if defined(_MSC_VER)
#   pragma warning (pop)
#endif //defined(_MSC_VER)

class PLBmp;

typedef std::vector<int> KernelVec;

class PLFilterGaussianBlur : public PLFilter {
    public:
        PLFilterGaussianBlur(double theRadius, unsigned theRealWidth, unsigned theRealHeight, double theSigma=1.0) :
            _myRadius(theRadius), _myRealWidth(theRealWidth), _myRealHeight(theRealHeight), _mySigma(theSigma)
        {
            calcKernel();
        }
    virtual ~PLFilterGaussianBlur() {};
    virtual void Apply(PLBmpBase * theSource, PLBmp * theDestination) const;
    
    private:
    void calcKernel() const;
    
    mutable double _mySigma;
    mutable double _myRadius;
    mutable int _myKernelWidth;
    mutable KernelVec _myKernel;
		unsigned _myRealWidth;
		unsigned _myRealHeight;
};

#endif
