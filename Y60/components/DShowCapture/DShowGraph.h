//============================================================================
//
// Copyright (C) 2005, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//============================================================================
//
//   $RCSfile: DShowGraph.h,v $
//
//   $Author: thomas $
//
//   $Revision: 1.5 $
//
//=============================================================================

#ifndef _ac_DShowCapture_DShowGraph_h_
#define _ac_DShowCapture_DShowGraph_h_

#include <dshow.h>
#include <qedit.h>
#include <string>
#include <vector>
#include "DXSampleGrabber.h"

#define SafeRelease(p) { if( (p) != 0 ) { (p)->Release(); (p)= NULL; } }

namespace y60 {
/*! @addtogroup Y60componentsDShowCapture */
/* @{ */
class DShowGraph {
public:
    bool createFilterGraph(IMoniker * videoDeive);
    DShowGraph();
    virtual ~DShowGraph();

    void setDesiredVideoFormat(int theWidth, int theHeight, int theFps, int theBits);

    // Automaticly find a capturing device to capture video
    void CaptureLive(int theIndex);

    // Init the given video device, but does NOT start the capture;
    bool initCaptureLive (IMoniker * videoDevice);

    // Destroy the whole Graph
    void Destroy() { destroyFilterGraph(); };

    // Pause the video
    void Pause() { m_pMediaControl->Pause(); }
    // Continue play the video
    void Play() ;
    // Stop the video
    void Stop()  { m_pMediaControl->Stop(); }
    LONGLONG Seek( int offset );
    // Seek in the video if the media type allows

    // this is only useful before the video graph is built
    // if you don't want to show the video, 
    // call it first thing after the object is constructed.
    void setVideoFlag(bool flag); 
    void unlockImage() {m_pTrackingCB->unlockImage();}
    unsigned char* lockImage() {return m_pTrackingCB->lockImage();}
    int getWidth() const {return m_pTrackingCB->getWidth();}
    int getHeight() const {return m_pTrackingCB->getHeight();}
    void setAnalogVideoFormat(void);
    void traceCrossbarInfo(IAMCrossbar *pXBar);
    void configCrossbar();
    std::vector<std::string> enumDevices();
protected:
    // Create the filter graph to render the video (capture or playback)
    virtual bool createFilterGraph(int theIndex = 0);
    // Release the filter graph
    virtual void destroyFilterGraph();
    // Start the playing
    void startGraph();
    // Stop playing
    void stopGraph();
    // automaticly find a usable capturing device
    HRESULT findCaptureDevice(IBaseFilter ** ppSrcFilter, int theDeviceIndex = 0);

    ///////////////////////////////////////////////////////////////////
    // Build the capture graph
    virtual bool buildCaptureGraph(IBaseFilter * filter = 0, int theIndex = 0);

    ///////////////////////////////////////////////////////////////////
    // The following 3 functions are for derivation if you need extra
    // filters in the graph, implement the 3 functions to do it
    ///////////////////////////////////////////////////////////////////
    // create and add extra filters, called in createFilterGraph() to
    // create extra filters into the graph
    virtual void    addExtraFilters();
    // release the resource related to the extra filters
    virtual void    releaseExtraFilters();
    // initialization the extra filters after the graph is finished
    virtual void	initExtraFilters();

    // Get the input or output pin to connect filters
    IPin* get_pin( IBaseFilter* pFilter, PIN_DIRECTION dir );

    // DirectShow interface pointers
    IGraphBuilder *			m_pGraphBuilder; // build render graph
    ICaptureGraphBuilder2 * m_pCaptureGraphBuilder2; // for capture
    IMediaControl*			m_pMediaControl; // MediaControl
    IFilterGraph*			m_pFilterGraph;  // Filter Graph
    IMediaSeeking*			m_pMediaSeeking; // Seeking interface
	// The interface to set callback function
	// ColorConverter is needed to convert other color to RGB24
	IBaseFilter *           m_pColorConv;
	// Grab filter which get image data for each frame
    IBaseFilter *			m_pGrabFilter;
	// The interface to set callback function
	ISampleGrabber *		m_pSampleGrabber;
    DXSampleGrabber *       m_pTrackingCB;

    bool			m_fIsScaling;   // scale the video to the display window
    //bool			m_fShowCaptureProperties; // Setup dialog before capture
    bool			m_fstartGraph; // has the video graph started;
    DWORD			m_dwGraphRegister;
    void	        removeGraphFromRot(DWORD pdwRegister);
    HRESULT         addGraphToRot(IUnknown *pUnkGraph, DWORD *pdwRegister);

    int				_myDesiredWidth, _myDesiredHeight, _myDesiredBits;
    int				_myDesiredFrameRate;
    std::string     _myDevice;
    AnalogVideoStandard _myDesiredVideoStandard;
    std::string     pinToString(long lType);

    HRESULT         selectVideoFormat();
    //private:
public:
    IBaseFilter *   m_pSrcFilter;
};
/* @} */
}

#endif // _ac_DShowCapture_DShowGraph_h_
