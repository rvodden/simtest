if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

cmake_minimum_required(VERSION 3.16)
project(simtest)
include(RCC)

file(GLOB SOURCES "src/*.[c,h]")

find_package (LibWebSockets REQUIRED)
find_package(LibUV REQUIRED)
find_package (simavr REQUIRED)
find_package (OpenSSL REQUIRED)
find_package (LibElf REQUIRED)

add_executable(simtest ${SOURCES})


target_include_directories(simtest PUBLIC ${SIMAVR_INCLUDE_DIR})
target_include_directories(simtest PUBLIC ${LIBWEBSOCKETS_INCLUDE_DIR})
target_include_directories(simtest PUBLIC ${OPENSSL_INCLUDE_DIR})
target_include_directories(simtest PUBLIC ${LIBUV_INCLUDE_DIR})
target_include_directories(simtest PUBLIC ${LIBELF_INCLUDE_DIR})

target_link_libraries(simtest ${SIMAVR_LIBRARIES})
target_link_libraries(simtest ${LIBWEBSOCKETS_LIBRARIES})
target_link_libraries(simtest OpenSSL::SSL)
target_link_libraries(simtest ${LIBELF_LIBRARIES} )
target_link_libraries(simtest ${LIBUV_LIBRARIES} )

rcc_add_resource_library(client resources/app.js resources/index.html resources/style.css)
target_link_libraries(simtest client)

configure_file(../firmware/target/main.elf main.elf)

if(${LINUX})
    target_link_libraries(simtest pthread)
endif()
