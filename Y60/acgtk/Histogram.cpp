#include "Histogram.h"

#include <asl/Logger.h>
#include <paintlib/pldebug.h>
#include <paintlib/Filter/pl2passscale.h>

#include <iostream>

using namespace std;

namespace acgtk {


class CData_UnsignedInt
{
public:
  typedef unsigned int _DataType[1];
  typedef _DataType* _RowType;
  class _Accumulator {
  public:
      _Accumulator ()
      {
        val = 0;
      };
      void Accumulate (long long Weight, _DataType &value)
      {
        val += (Weight * (long long)(value[0]));
          // AC_WARNING << "adding " << Weight << "x" << value[0] << " -> " << val;
      };
      void Store (_DataType &value)
      {
          // AC_WARNING << "storing val=" << val;
        value [0] = (unsigned int) ((val + 128)/256);
      };
      long long val;
  };
};

const static float MIN_WINDOW_WIDTH(10.0);

Histogram::Histogram() :
    Gtk::DrawingArea(),
    _myMode(MODE_CENTER_WIDTH),
    _myLogarithmicScaleFlag(true),
    _myDrawWindowFlag(false),
    _myDrawCenterFlag(false)
{
    set_size_request(100, 50);
    Gdk::EventMask myFlags = get_events();
    myFlags |= Gdk::POINTER_MOTION_MASK;
    myFlags |= Gdk::BUTTON_PRESS_MASK;
    myFlags |= Gdk::BUTTON_RELEASE_MASK;
    set_events(myFlags);
}

Histogram::~Histogram() {
}


void
Histogram::on_realize() {
  // We need to call the base on_realize()
  Gtk::DrawingArea::on_realize();

  // Now we can allocate any additional resources we need
  _myWindow = get_window();
  _myWindow->clear();
}

/*
bool 
Histogram::on_button_press_event(GdkEventButton * theEvent) {
    //cerr << "Histogram::on_button_press_event()" << endl;
    _myLogarithmicScaleFlag = ! _myLogarithmicScaleFlag;
    queue_draw();
    return true;
}
bool 
Histogram::on_button_release_event(GdkEventButton * theEvent) {
    //cerr << "Histogram::on_button_release_event()" << endl;
    return true;
}

bool 
Histogram::on_motion_notify_event(GdkEventMotion * theEvent) {
    return true;
}
*/

bool 
Histogram::on_configure_event(GdkEventConfigure * theEvent) {
    //cerr << "Histogram::on_configure_event()" << endl;
    DrawingArea::on_configure_event(theEvent);
    rebuildBins();
    return true;
}

bool
Histogram::on_expose_event(GdkEventExpose * theEvent) {
    //cerr << "Histogram::on_expose_event()" << endl;
    DrawingArea::on_expose_event(theEvent);

    _myWindow->clear();

    int myWidth    = get_allocation().get_width();
    int myHeight   = get_allocation().get_height();
    int myBinCount = _myBins.size();

    // TODO: handle myBarWidth < 1.0
    float myBarWidth = float(myWidth) / myBinCount;
    
    Glib::RefPtr<Gdk::GC> myGC = get_style()->get_black_gc();

    unsigned myCount;
    for (unsigned i = 0; i < _myBins.size(); ++i) {
        myCount = convertSampleCountToScreenPos(_myBins[i], _myMaxCount);
        _myWindow->draw_rectangle(myGC, true, int(i * myBarWidth), myHeight - myCount, 
                int(ceilf(myBarWidth)), myCount);
    }

    if (_myDrawWindowFlag) {
        int myXStart;
        int myXEnd;
        if (_myMode == MODE_CENTER_WIDTH) {
            myXStart = convertValueToScreenPos(_myWindowCenter - (0.5 * _myWindowWidth));
            myXEnd   = convertValueToScreenPos(_myWindowCenter + (0.5 * _myWindowWidth));
        } else {
            myXStart = convertValueToScreenPos(_myLower);
            myXEnd   = convertValueToScreenPos(_myUpper);
        }

        myGC = get_style()->get_dark_gc(get_state()),
             myGC->set_function(Gdk::XOR);
        _myWindow->draw_rectangle(myGC, true, myXStart, 0, myXEnd - myXStart, myHeight);
        myGC->set_function(Gdk::COPY);
    }

    if (_myDrawCenterFlag && _myMode == MODE_CENTER_WIDTH) {
        int myXPos = convertValueToScreenPos(_myWindowCenter);
        myGC = get_style()->get_white_gc();
        myGC->set_function(Gdk::XOR);
        _myWindow->draw_line(myGC, myXPos, 0, myXPos, myHeight);
        myGC->set_function(Gdk::COPY);
    }

    return true;
}

void
Histogram::rebuildBins() {
    int myWidth = get_allocation().get_width();
    if (myWidth < _mySampleData.size()) {
        _myBins.clear();
        _myBins.resize(myWidth);
        
        // set up some pointers to simulate paintlib's linearray
        unsigned int * mySrcLinePtr = (&_mySampleData[0]); 
        unsigned int * myDestLinePtr = (&_myBins[0]); 
        // set up the scaler
        PLBilinearContribDef f(0.64);
        C2PassScale <CData_UnsignedInt> sS(f);
        // and start it 
        sS.Scale( (CData_UnsignedInt::_RowType*)(&mySrcLinePtr), _mySampleData.size(), 1,
                  (CData_UnsignedInt::_RowType*)(&myDestLinePtr), _myBins.size(), 1);
    } else {
        _myBins = _mySampleData;
    }
    _myMaxCount = findMaxCount();
}

unsigned
Histogram::findMaxCount() {
    unsigned myMax = 0;
    for (unsigned i = 0; i < _myBins.size(); ++i) {
        if (_myBins[i] > myMax) {
            myMax = _myBins[i];
        }
    }
    return int( 1.1 * myMax); // make 10% headroom
}

int 
Histogram::convertSampleCountToScreenPos(int theSampleCount, int theMaxSampleCount) {
    int myHeight   = get_allocation().get_height();
    if (_myLogarithmicScaleFlag) {
        return int( log( float(theSampleCount)) * myHeight / log(float(theMaxSampleCount)));
    } else {
        return theSampleCount * myHeight / theMaxSampleCount;
    }
}

void 
Histogram::setHistogram(const std::vector<unsigned> & theSamples) {
    _mySampleData = theSamples;
    rebuildBins();
    queue_draw();
}
void 
Histogram::setShowWindow(bool theFlag) {
    _myDrawWindowFlag = theFlag;
    queue_draw();
}

bool
Histogram::getShowWindow() const {
    return _myDrawWindowFlag;
}

void
Histogram::setShowWindowCenter(bool theFlag) {
    _myDrawCenterFlag = theFlag;
    queue_draw();
}

bool
Histogram::getShowWindowCenter() const {
    return _myDrawCenterFlag;
}

void 
Histogram::setWindowCenter(float theValue) {
    _myWindowCenter = theValue;
    queue_draw();
}

void 
Histogram::setWindowWidth(float theValue) {
    _myWindowWidth = theValue;
    queue_draw();
}

void 
Histogram::setLower(float theValue) {
    _myLower = theValue;
    queue_draw();
}

void 
Histogram::setUpper(float theValue) {
    _myUpper = theValue;
    queue_draw();
}

void 
Histogram::setValueRange(const asl::Vector2f & theRange) {
    _myValueRange = theRange;
}

const asl::Vector2f &
Histogram::getValueRange() const {
    return _myValueRange;
}

void 
Histogram::setLogarithmicScale(bool theFlag) {
    _myLogarithmicScaleFlag = theFlag;
    queue_draw();
}

bool
Histogram::getLogarithmicScale() const {
    return _myLogarithmicScaleFlag;
}

int 
Histogram::convertValueToScreenPos(const float & theValue) {
    int myWidth = get_allocation().get_width();
    return int((theValue - _myValueRange[0])* float(myWidth) / (_myValueRange[1] - _myValueRange[0]));
}


} // end of namespace
