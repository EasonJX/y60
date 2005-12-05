//=============================================================================
// Copyright (C) 1993-2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: ACColumnRecord.cpp,v $
//   $Author: martin $
//   $Revision: 1.1 $
//   $Date: 2004/11/25 11:45:08 $
//
//
//=============================================================================

#include "ACColumnRecord.h"

ACColumnRecord::ACColumnRecord(unsigned int theColCount) {
    add(_myRecId);
    _myColumns.resize(theColCount);
    for (int i = 0; i < theColCount; ++i) {
        add(_myColumns[i]);
    }
}


