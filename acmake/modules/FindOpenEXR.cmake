if( WIN32 )

    find_path(OPENEXR_INCLUDE_DIR ImfRgbaFile.h PATH_SUFFIXES OpenEXR)

    set(OPENEXR_ALL_LIBRARIES Half Iex IlmImf IlmThread Imath )
    foreach( SUBLIB ${OPENEXR_ALL_LIBRARIES} )
       mark_as_advanced( OPENEXR_SUBLIB_${SUBLIB} )
       find_library( OPENEXR_SUBLIB_${SUBLIB} NAMES ${SUBLIB} ) 
       if ( OPENEXR_SUBLIB_${SUBLIB}-NOTFOUND )
           set( OPENEXR-NOTFOUND TRUE )
           break()
       else ( OPENEXR_SUBLIB_${SUBLIB}-NOTFOUND )
           list( APPEND OPENEXR_LIBRARIES ${OPENEXR_SUBLIB_${SUBLIB}} )
       endif ( OPENEXR_SUBLIB_${SUBLIB}-NOTFOUND )
    endforeach( SUBLIB ${OPENEXR_ALL_LIBRARIES} )
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(OPENEXR DEFAULT_MSG
            OPENEXR_LIBRARIES OPENEXR_INCLUDE_DIR)
    set(OPENEXR_DEFINITIONS -DOPENEXR_DLL )
    mark_as_advanced( OPENEXR_DEFINITIONS OPENEXR_INCLUDE_DIR )
else( WIN32 )
    pkg_search_module( OPENEXR REQUIRED OpenEXR )
endif( WIN32 )

