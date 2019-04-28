SET (SIMAVR_FOUND FALSE)

find_path (SIMAVR_INCLUDE_DIR NAMES sim_avr.h sim_elf.h sim_irq.h
    HINTS /usr/include/simavr /usr/local/include/simavr   
)

find_library ( SIMAVR_LIBRARIES NAMES simavr
    HINTS /usr/local/lib /usr/lib 
)

message(STATUS "SIMAVR_LIBRARIES: [${SIMAVR_LIBRARIES}]")

IF ( SIMAVR_INCLUDE_DIR )
    IF ( SIMAVR_LIBRARIES )
        SET ( SIMAVR_FOUND TRUE )
    ENDIF ( SIMAVR_LIBRARIES )
ENDIF ( SIMAVR_INCLUDE_DIR )

IF ( SIMAVR_FOUND )
    MARK_AS_ADVANCED(
        SIMAVR_LIBRARY_DIR
        SIMAVR_INCLUDE_DIR
        SIMAVR_LIBRARIES
    )
ENDIF ( SIMAVR_FOUND )

find_package_handle_standard_args(simavr
	DEFAULT_MSG
	SIMAVR_INCLUDE_DIR
	SIMAVR_LIBRARIES
)
