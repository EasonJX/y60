# This file resolves the dependencies of Y60

include(FindPkgConfig)

# art+com standard library
list(APPEND REQ_PACKAGES ASL)

# netsrc dependencies
list(APPEND REQ_PACKAGES Paintlib)
list(APPEND REQ_PACKAGES ACSDLTTF)
list(APPEND REQ_PACKAGES OSCPACK )

# javascript implementation
if (NOT Y60_WITH_TRACEMONKEY)
    list(APPEND REQ_PACKAGES Spidermonkey)
else (NOT Y60_WITH_TRACEMONKEY)
    list(APPEND REQ_PACKAGES MOZJS)
endif (NOT Y60_WITH_TRACEMONKEY)

# external dependencies

# boost 
set(BOOST_MIN_VERSION "1.48.0")
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED)

list(APPEND REQ_PACKAGES SDL GLEW NvidiaCg OpenEXR GLIB2)
list(APPEND OPT_PACKAGES Maya)

if(APPLE AND SDL_LIBRARY MATCHES ".*\\.framework.*")
    set(Y60_SDL_IS_FRAMEWORK YES)
    set(Y60_SDLMAIN y60_sdlmain)
endif(APPLE AND SDL_LIBRARY MATCHES ".*\\.framework.*")

# gtk and consorts
list(APPEND OPT_PACKAGES GTKMM)
list(APPEND OPT_PACKAGES GTKGLEXT)
list(APPEND OPT_PKGCONFIG CAIROMM cairomm-1.0)

# media codecs
list(APPEND REQ_PACKAGES FFMPEG)

# input device support
list(APPEND OPT_PACKAGES TUIO)

# crypto++
list(APPEND OPT_PACKAGES Crypto++)

# opencv lib
list(APPEND OPT_PACKAGES OpenCV)

if(WIN32)

    # pthread emulation library
    list(APPEND REQ_PACKAGES PthreadVC)

    # we need some flavour of ATL for the WMV and WMA components
    list(APPEND REQ_PACKAGES ATL)

    # we always need to add FakeATL here so it will
    # be in the dependency list in the installer
    #  XXX: dirty hack? does anything bad come from this?
    list(APPEND REQ_PACKAGES FakeATL)

    # cairo backend
    list(APPEND OPT_PKGCONFIG CAIRO cairo-win32)
    list(APPEND OPT_PKGCONFIG PANGO pango)
    list(APPEND OPT_PKGCONFIG PANGOCAIRO pangocairo)

    # windows media sdk
    list(APPEND REQ_PACKAGES Wmsdk)

    # cinema 4d
    list(APPEND OPT_PACKAGES C4D)

    # DirectShow GUIDs (comes with Windows SDK)
    list(APPEND REQ_PACKAGES Strmiids)

    # alled gigabit camera api lib
    list(APPEND OPT_PACKAGES AlliedGigabit)

elseif(LINUX)

    # cairo backend
    list(APPEND OPT_PKGCONFIG CAIRO cairo-xlib)
    list(APPEND OPT_PKGCONFIG PANGO pango)
    list(APPEND OPT_PKGCONFIG PANGOCAIRO pangocairo)

elseif(OSX)

    # cairo backend
    list(APPEND OPT_PKGCONFIG CAIRO cairo-quartz)
    list(APPEND OPT_PKGCONFIG PANGO pango)
    list(APPEND OPT_PKGCONFIG PANGOCAIRO pangocairo)

    # cocoa ui framework
    find_library( COCOA_LIBRARIES NAMES Cocoa)
    mark_as_advanced( COCOA_LIBRARIES )
    
endif(WIN32)

