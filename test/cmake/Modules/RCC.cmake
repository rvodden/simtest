if(_RCC_GENERATE_MODE)
    message(STATUS "Generating file ${OUTPUT_FILE}")
	
	# Read in the digits
    file(READ "${INPUT_FILE}" bytes HEX)
   	string(APPEND bytes 00) 
	
    # Format each pair into a character literal
    string(REGEX REPLACE "(..)" "'\\\\x\\1', " chars "${bytes}")
	set(intermediate_content "static const char file_array[] = { ${chars} }; 
const char* ${SYMBOL}_begin = file_array;
const char* ${SYMBOL}_end = file_array + sizeof(file_array);
const int ${SYMBOL}_size = sizeof(file_array);
")
	
	file(WRITE "${OUTPUT_FILE}" "${intermediate_content}")
    return()
endif()

if(COMMAND rcc_add_resource_library)
    # RCC has already been included! Don't do anything
    return()
endif()

set(this_script "${CMAKE_CURRENT_LIST_FILE}")

function(rcc_add_resource_library name)
        
    set(sources)
    foreach(input IN LISTS ARGN)
        get_filename_component(abs_input "${input}" ABSOLUTE)

        # Generate a filename based on the input filename that we can put in
        # the intermediate directory.
        file(RELATIVE_PATH relpath "${CMAKE_CURRENT_SOURCE_DIR}" "${abs_input}")
        if(relpath MATCHES "^\\.\\.")
            # For now we just error on files that exist outside of the soure dir.
            message(SEND_ERROR "Cannot add file '${input}': File must be in a subdirectory of the source directory")
            continue()
        endif()
        get_filename_component(abspath "${CMAKE_CURRENT_BINARY_DIR}/${name}-intermediate/${relpath}.c" ABSOLUTE)
        
        # Generate the rule for the intermediate source file
        _rcc_encode_fpath(sym "${input}")
        MESSAGE(STATUS "Processing: ${sym}: ${abspath}")
        _rcc_generate_intermediate_c("${abspath}" "${abs_input}" "${sym}")
        list(APPEND sources "${abspath}")
    endforeach()

    add_library(${name} STATIC ${sources})
    target_link_libraries(${name} INTERFACE rcc-base)
endfunction()

# Let'}s generate the primary include file
set(header_content [==[

    #ifndef __RCC_H__
    #define __RCC_H__

    typedef struct rcc_resource {
        char* start;
        char* end;
        int size;
    } rcc_resource;

    extern struct rcc_resource*
    rcc_open(
        char* name
        );

    #endif // __RCC_H__
    
    ]==])

# Create the directory for the generated header file to hang out in
set(RCC_INCLUDE_DIR "${CMAKE_BINARY_DIR}/_rcc/include" CACHE INTERNAL "Directory for RCC include files")
set(rcc_header_path "${RCC_INCLUDE_DIR}/rcc/rcc.h" CACHE INTERNAL "Location of RCC Header file")
file(MAKE_DIRECTORY "${RCC_INCLUDE_DIR}/rcc")
file(GENERATE OUTPUT "${rcc_header_path}" CONTENT "${header_content}")

add_library(rcc-base INTERFACE)
target_include_directories(rcc-base INTERFACE "${RCC_INCLUDE_DIR}")
add_library(rcc-base::base ALIAS rcc-base)

function(_rcc_generate_intermediate_c outfile infile symbol)
    add_custom_command(
        # This is the file we will generate
        OUTPUT "${outfile}"
        # These are the primary files that affect the output
        DEPENDS "${infile}" "${this_script}"
        COMMAND
            "${CMAKE_COMMAND}"
                -D_RCC_GENERATE_MODE=TRUE
                -DSYMBOL=${symbol}
                "-DINPUT_FILE=${infile}"
                "-DOUTPUT_FILE=${outfile}"
                -P "${this_script}"
        COMMENT "Generating intermediate file for ${infile}"
    )
endfunction()

function(_rcc_encode_fpath var fpath)
    string(MAKE_C_IDENTIFIER "${fpath}" ident)
    set(${var} r_${ident} PARENT_SCOPE)
endfunction()
