if( WIN32 )

    find_path(OPENEXR_INCLUDE_DIR ImfRgbaFile.h PATH_SUFFIXES OpenEXR)

    set(OPENEXR_ALL_LIBRARIES Half Iex IlmImf IlmThread Imath )
    foreach( SUBLIB ${OPENEXR_ALL_LIBRARIES} )
       mark_as_advanced( THIS_SUBLIB )
       find_library( THIS_SUBLIB NAMES ${SUBLIB} ) 
       if ( THIS_SUBLIB-NOTFOUND )
           set( OPENEXR-NOTFOUND TRUE )
           break()
       else ( THIS_SUBLIB-NOTFOUND )
           list( APPEND OPENEXR_LIBRARIES ${THIS_SUBLIB} )
           set( OPENEXR_LIBRARY_DIRS ${THIS_SUBLIB_LIBRARY_DIR} )
       endif ( THIS_SUBLIB-NOTFOUND )
    endforeach( SUBLIB ${OPENEXR_ALL_LIBRARIES} )
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(OPENEXR DEFAULT_MSG
            OPENEXR_LIBRARIES OPENEXR_INCLUDE_DIRS)
    set(OPENEXR_DEFINITIONS -DOPENEXR_DLL )
    mark_as_advanced( OPENEXR_DEFINITIONS OPENEXR_INCLUDE_DIRS )
else( WIN32 )
    pkg_search_module( OPENEXR REQUIRED OpenEXR )
endif( WIN32 )
