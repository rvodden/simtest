if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

cmake_minimum_required(VERSION 2.6)
project(simtest)

file(GLOB SOURCES "src/*.[c,h]")

find_package (LibWebSockets)
find_package (simavr)
find_package (OpenSSL)
find_package (LibElf)

add_executable(simtest ${SOURCES})

target_include_directories(simtest PUBLIC ${SIMAVR_INCLUDE_DIR})
target_include_directories(simtest PUBLIC ${LIBWEBSOCKETS_INCLUDE_DIR})
target_include_directories(simtest PUBLIC ${OPENSSL_INCLUDE_DIR})

target_link_libraries(simtest ${SIMAVR_LIBRARIES} ${OPENSSL_LIBRARIES} ${LIBWEBSOCKETS_LIBRARIES})
target_link_libraries(simtest ${LIBELF_LIBRARIES} )

configure_file(../firmware/target/main.elf main.elf)

if(LINUX)
    target_link_libraries(simtest pthread)
endif()
