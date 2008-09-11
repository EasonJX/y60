#include "GStreamerCapturePlugin.h"
#include <asl/base/Auto.h>

extern "C"
EXPORT asl::PlugInBase * GStreamerCapturePlugin_instantiatePlugIn(asl::DLHandle myDLHandle) {
	return new y60::GStreamerCapturePlugin(myDLHandle);
}

namespace y60 {

    const int MAX_PORTS = 4;
    
    GStreamerCapturePlugin::GStreamerCapturePlugin(asl::DLHandle theDLHandle) : PlugInBase(theDLHandle), _myBufferSize(0)
    {
        //AC_PRINT << "gstreamercaptureplugin ctr: " << this;
        memset( _myBuffer, 0, sizeof( _myBuffer ));

    }

    GStreamerCapturePlugin::~GStreamerCapturePlugin() {
    }

    void GStreamerCapturePlugin::handoff( GstElement *fakesrc, GstBuffer *buffer, 
                                          GstPad *pad) {

        GstCaps* myBufferCaps = gst_buffer_get_caps( buffer );
        //AC_PRINT << "myBufferCaps: " << gst_caps_to_string( myBufferCaps );


        int buffer_size = GST_BUFFER_SIZE (buffer);
        guint8* buffer_data = GST_BUFFER_DATA (buffer);

        assert( buffer_size < sizeof( _myBuffer ) );
        asl::AutoLocker<asl::ThreadLock> myAutoLocker(_myBufferLock);
        std::copy( buffer_data, buffer_data + buffer_size, _myBuffer ); 
        _myBufferSize = buffer_size;
        //AC_PRINT << "handoff: " 
        //         << getpid() << " "
        //         << this << " "
        //         << (void*)(&_myBuffer[0]) << " " 
        //         << _myBufferSize << " " 
        //         << std::hex << int(_myBuffer[1000]);




    }

    gboolean GStreamerCapturePlugin::buscallback( GstBus *bus, GstMessage* message ) {
        g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

        switch (GST_MESSAGE_TYPE (message)) {
            case GST_MESSAGE_ERROR: {
                                        GError *err;
                                        gchar *debug;

                                        gst_message_parse_error (message, &err, &debug);
                                        g_print ("Error: %s\n", err->message);
                                        g_error_free (err);
                                        g_free (debug);

                                        break;
                                    }
            case GST_MESSAGE_EOS:
                                    /* end-of-stream */
                                    break;
            default:
                                    /* unhandled message */
                                    break;
        }

        /* we want to be notified again the next time there is a message
         * on the bus, so returning TRUE (FALSE means we want to stop watching
         * for messages on the bus and our callback should not be called again)
         */
        return TRUE;
    }

    void GStreamerCapturePlugin::readFrame(dom::ResizeableRasterPtr theTargetRaster) {
        assert (_myBufferSize <= theTargetRaster->pixels().size());
        theTargetRaster->resize(getFrameWidth(), getFrameHeight());
        asl::AutoLocker<asl::ThreadLock> myAutoLocker(_myBufferLock);
        memcpy(theTargetRaster->pixels().begin(), _myBuffer, _myBufferSize);
        //AC_PRINT << "readframe: " 
        //         << getpid() << " "
        //         << this << " "
        //         << (void*)(&_myBuffer[0]) << " " 
        //         << _myBufferSize << " " 
        //         << std::hex << int(_myBuffer[1000]);
        //_myBufferSize = 0;
    }
    
    bool dumpPads( GstElement* theElement ) {
        
        GstIterator* myPadIterator =  gst_element_iterate_src_pads( theElement );
        GstPad* myPad = NULL;

        while (true) {
            switch (gst_iterator_next( myPadIterator, (void**)&myPad )) {
                case GST_ITERATOR_OK:
                {
                    GstCaps* myCaps = gst_pad_get_caps(myPad);
                    //AC_PRINT << "caps: " << gst_caps_to_string( myCaps );
                    gst_object_unref( myPad );
                }
                    break;
                case GST_ITERATOR_DONE:
                    return true;
                default: 
                    return false;
            }
        }
    }

    void GStreamerCapturePlugin::load(const std::string & theFilename) {
        // init gstreamer (with fake arguments)
        //
        gst_init( NULL, NULL );

        // grab version information
        // 
        const gchar *nano_str;
        guint major, minor, micro, nano;
      
        gst_version (&major, &minor, &micro, &nano);
      
        if (nano == 1)
          nano_str = "(CVS)";
        else if (nano == 2)
          nano_str = "(Prerelease)";
        else
          nano_str = "";
        
        AC_INFO << "Plugging GStreamer Plugin using GStreamer version "
                << major << "." << minor << "." << micro << "." << std::string(nano_str);

        GstElement *myPipeline;
        myPipeline = gst_pipeline_new( "Capture" );

        GstBus *myBus;
        myBus = gst_pipeline_get_bus( GST_PIPELINE( myPipeline ));
        gst_bus_add_watch( myBus, cb_buscallback, gpointer(this) );
        gst_object_unref( myBus );

        GstElement *mySource, *myConverter, *myFakeSink, *myFilter;
        mySource = gst_element_factory_make( "v4l2src", "source" );
        myConverter = gst_element_factory_make( "ffmpegcolorspace", "conv" );
        myFilter = gst_element_factory_make ("capsfilter", "filter");
        myFakeSink = gst_element_factory_make( "fakesink", "sink" );
        GstCaps* myCaps = gst_caps_new_simple( "video/x-raw-rgb",
                                               NULL);
        g_object_set( G_OBJECT( myFilter ), "caps", myCaps, NULL ); 


        gst_bin_add_many( GST_BIN( myPipeline ), mySource, myConverter, myFilter, myFakeSink, NULL );

        if (!gst_element_link_many( mySource, myConverter, myFilter, myFakeSink, NULL )) {
            g_warning( "Failed to link elements" );
        }

        dumpPads( mySource );

        g_object_set( G_OBJECT( myFakeSink ), "signal-handoffs", TRUE );
        g_signal_connect( myFakeSink, "handoff", 
                          G_CALLBACK( cb_handoff ), 
                          this );

        gst_element_set_state( myPipeline, GST_STATE_PLAYING );
            //AC_PRINT << theFilename;
        setFrameWidth(640);
        setFrameHeight(480);
        setPixelFormat(RGB);
    }


    std::string GStreamerCapturePlugin::canDecode(const std::string & theUrl, 
                                                  asl::Ptr<asl::ReadableStreamHandle> 
                                                    theStream)
    {
        if (theUrl.find("video://") != std::string::npos) {
            return "video/camera";
        } else {
            return "video/camera";
        }
    }

    asl::Ptr<CaptureDevice> GStreamerCapturePlugin::instance() const {
        return asl::Ptr<CaptureDevice>(new GStreamerCapturePlugin(getDLHandle()));
    }

    void GStreamerCapturePlugin::stopCapture() {
        CaptureDevice::stopCapture();
    }
    
    void GStreamerCapturePlugin::startCapture() {
        CaptureDevice::startCapture();
    }

    void GStreamerCapturePlugin::pauseCapture() {
        CaptureDevice::pauseCapture();
    }
}
