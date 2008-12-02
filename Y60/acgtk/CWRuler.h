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

#ifndef ACGTK_CW_RULER_INCLUDED
#define ACGTK_CW_RULER_INCLUDED

#include <asl/math/Vector234.h>

#if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4413 4244)
#endif //defined(_MSC_VER)
#include <gtkmm/drawingarea.h>
#include <sigc++/sigc++.h>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif //defined(_MSC_VER)

namespace acgtk {

class CWRuler : public Gtk::DrawingArea {
    public:
        enum Mode {
            MODE_THRESHOLD, // [DS] ... think this is deprecated
            MODE_CENTER_WIDTH,
            MODE_LOWER_UPPER
        };
        CWRuler(Mode theMode = MODE_CENTER_WIDTH);
        virtual ~CWRuler();

        void setValueRange(const asl::Vector2f & theValueRange);
        const asl::Vector2f & getValueRange() const;

        void setWindowCenter(float theCenter);
        float getWindowCenter() const;

        void setWindowWidth(float theWidth);
        float getWindowWidth() const;

        void setLower(float theLower);
        float getLower() const;

        void setUpper(float theUpper);
        float getUpper() const;

        void setMode(Mode theMode);
        Mode getMode() const;

        sigc::signal<void, float> signal_center_changed() const { return _myCenterChangedSignal;}
        sigc::signal<void, float> signal_width_changed() const { return _myWidthChangedSignal;}
        sigc::signal<void, float> signal_lower_changed() const { return _myLowerChangedSignal;}
        sigc::signal<void, float> signal_upper_changed() const { return _myUpperChangedSignal;}

    protected:
        virtual bool on_expose_event(GdkEventExpose * theEvent);
        virtual bool on_button_press_event(GdkEventButton * theEvent);
        virtual bool on_button_release_event(GdkEventButton * theEvent);
        virtual bool on_motion_notify_event(GdkEventMotion * theEvent);
        void on_realize();
    private:
        void drawMarker(float thePosition, Glib::RefPtr<Gdk::GC> theGC);
        int convertValueToScreenPos(const float & theValue);
        float convertScreenPosToValue(const int & theScreenPos);
        bool intersectWithMarker(GdkEventButton * theEvent, int theMarkerPos);
   
        enum State {
            IDLE,
            CHANGE_CENTER,
            CHANGE_WIDTH_LEFT,
            CHANGE_WIDTH_RIGHT
        };

        Mode          _myMode;
        State         _myState;
        asl::Vector2f _myValueRange;
        float        _myWindowCenter;
        float        _myWindowWidth;
        float        _myLower;
        float        _myUpper;
        
        Glib::RefPtr<Gdk::Window>  _myWindow;
        sigc::signal<void, float> _myCenterChangedSignal;
        sigc::signal<void, float> _myWidthChangedSignal;
        sigc::signal<void, float> _myLowerChangedSignal;
        sigc::signal<void, float> _myUpperChangedSignal;
};

}


#endif // ACGTK_CW_RULER_INCLUDED
