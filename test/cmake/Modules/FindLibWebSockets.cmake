# This module tries to find libWebsockets library and include files
#
# LIBWEBSOCKETS_INCLUDE_DIR, path where to find libwebsockets.h
# LIBWEBSOCKETS_LIBRARY_DIR, path where to find libwebsockets.so
# LIBWEBSOCKETS_LIBRARIES, the library to link against
# LIBWEBSOCKETS_FOUND, If false, do not try to use libWebSockets
#
# This currently works probably only for Linux

include(FindPackageHandleStandardArgs)
SET ( LIBWEBSOCKETS_FOUND FALSE )

find_path ( LIBWEBSOCKETS_INCLUDE_DIR NAMES libwebsockets.h
    HINTS /opt/libwebsockets/include /usr/local/include /usr/include )

find_library ( LIBWEBSOCKETS_LIBRARIES NAMES websockets
    HINTS /opt/libwebsockets/lib /usr/local/lib /usr/lib 
)

get_filename_component( LIBWEBSOCKETS_LIBRARY_DIR ${LIBWEBSOCKETS_LIBRARIES} PATH )

IF ( LIBWEBSOCKETS_INCLUDE_DIR )
    IF ( LIBWEBSOCKETS_LIBRARIES )
        SET ( LIBWEBSOCKETS_FOUND TRUE )
    ENDIF ( LIBWEBSOCKETS_LIBRARIES )
ENDIF ( LIBWEBSOCKETS_INCLUDE_DIR )


IF ( LIBWEBSOCKETS_FOUND )
    MARK_AS_ADVANCED(
        LIBWEBSOCKETS_LIBRARY_DIR
        LIBWEBSOCKETS_INCLUDE_DIR
        LIBWEBSOCKETS_LIBRARIES
    )
ENDIF ( LIBWEBSOCKETS_FOUND )

find_package_handle_standard_args(LibWebSockets
	DEFAULT_MSG
	LIBWEBSOCKETS_INCLUDE_DIR
	LIBWEBSOCKETS_LIBRARIES)
