cmake_minimum_required(VERSION 3.15)

project(sferamondo CXX)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")

if (NOT "${CMAKE_GENERATOR}" MATCHES "^Visual Studio.*")
    # set CMAKE_BUILD_TYPE if not already set
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE "Release")
    endif ()
    message("-- build_type: ${CMAKE_BUILD_TYPE}")

    if ("${CMAKE_GENERATOR}" MATCHES "Make")
        # when using 'make' we create a 'cleanup' target to invoke 'make clean'
        # and on which manual cleanup targets can be defined
        add_custom_target(cleanup
            COMMAND ${CMAKE_MAKE_PROGRAM} clean
            WORKING_DIRECTORY ${CMAKE_CURRENT_DIR}
        )
    endif ()
endif ()

include(FindPkgConfig)

#include("cmake/Compiler.cmake")
#include("cmake/Conan.cmake")
#include("cmake/SetupProtobuf.cmake")
#include("cmake/SetupGRPC.cmake")
#include("cmake/ProtoAutogen.cmake")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_subdirectory(src)

