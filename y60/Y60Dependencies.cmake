# This file resolves the dependencies of Y60

include(AcPlatform)

include(FindPkgConfig)

list(APPEND REQ_PACKAGES ASL)

# exported dependencies
if(NOT PAINTLIB_INTEGRATED_BUILD)
    list(APPEND REQ_PACKAGES Paintlib)
endif(NOT PAINTLIB_INTEGRATED_BUILD)
# if(NOT SPIDERMONKEY_INTEGRATED_BUILD)
#    list(APPEND REQ_PACKAGES Spidermonkey)
# endif(NOT SPIDERMONKEY_INTEGRATED_BUILD)
if(NOT ACSDLTTF_INTEGRATED_BUILD)
    list(APPEND REQ_PACKAGES ACSDLTTF)
endif(NOT ACSDLTTF_INTEGRATED_BUILD)

# external dependencies
list(APPEND REQ_PACKAGES SDL GLEW NvidiaCg OpenEXR GLIB2 MOZJS)
list(APPEND OPT_PACKAGES Maya C4D)

list(APPEND REQ_PKGCONFIG GTKMM gtkmm-2.4)
list(APPEND REQ_PKGCONFIG GLADEMM libglademm-2.4)
list(APPEND REQ_PKGCONFIG GTKGLEXT gtkglext-1.0)

if(WIN32)
    list(APPEND REQ_PACKAGES PthreadVC)
    list(APPEND REQ_PKGCONFIG CAIRO cairo-win32)
elseif(LINUX)
    list(APPEND REQ_PKGCONFIG CAIRO cairo-xlib)
elseif(UNIX AND APPLE)
    list(APPEND REQ_PKGCONFIG CAIRO cairo-quartz)
endif(WIN32)

list(APPEND REQ_PACKAGES FFMPEG )

if( OSX )
    find_library( COCOA_LIBRARIES NAMES Cocoa)
    mark_as_advanced( COCOA_LIBRARIES )
endif( OSX )

