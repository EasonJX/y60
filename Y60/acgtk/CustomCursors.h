//=============================================================================
// Copyright (C) 2004, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile: SliceViewer.js,v $
//   $Author: danielk $
//   $Revision: 1.97 $
//   $Date: 2005/04/29 13:49:59 $
//
//=============================================================================

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4413 4244 4512)
#endif //defined(_MSC_VER)
#include <gdkmm.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif //defined(_MSC_VER)

#include <iostream>

using namespace std;

namespace acgtk {

class CustomCursors {
    public:
        static void init();

        static Gdk::Cursor AC_ADD_POINT; 
        static Gdk::Cursor AC_EDIT_ANGLE; 
        static Gdk::Cursor AC_EDIT_ANGLE1; 
        static Gdk::Cursor AC_EDIT_ANGLE2; 
        static Gdk::Cursor AC_EDIT_ANGLE3; 

    private:
};

Gdk::Cursor createCustomCursor(unsigned char * theBitmap, unsigned char * theMask,
        int theWidth, int theHeight, int theHotX, int theHotY);
} // end of namespace
