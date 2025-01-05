
# ctest directory CMakeLists.txt for libmodbus
# Copyright © Pascal JEAN <epsilonrt@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause


configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/unit-test.h.in 
                ${CMAKE_CURRENT_BINARY_DIR}/unit-test.h @ONLY)

include(FetchContent)
# Download and make the Unit test framework available.
FetchContent_Declare(
    unity
    GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity
    GIT_TAG        v2.5.2
    SOURCE_DIR 3rdparty/unity

)
add_definitions(-DUNITY_INCLUDE_PRINT_FORMATTED)
FetchContent_MakeAvailable(unity)



# Collect a list of all the test files in the test directory.
set(ABSOLUTE_TEST_DIRECTORY_PATH ${CMAKE_SOURCE_DIR}/${TEST_DIRECTORY})
# cmake_print_variables(ABSOLUTE_TEST_DIRECTORY_PATH)
# cmake_print_variables(PROJECT_SHORTNAME)

file(GLOB_RECURSE TEST_FILES RELATIVE ${ABSOLUTE_TEST_DIRECTORY_PATH} ${TEST_FILE_PREFIX}*)

# Log the test files that were found.
message (STATUS "Discovered test files")
foreach(f ${TEST_FILES})
    message(STATUS "    " ${f})
endforeach ()

# Add a new executable and test for each test file discovered.
foreach (TEST_FILE ${TEST_FILES})

    # Remove the test filename prefix to get the path to the corresponding source file.
    string(REGEX REPLACE ${TEST_FILE_PREFIX} "" SOURCE_FILE ${TEST_FILE})
    cmake_path(REMOVE_EXTENSION SOURCE_FILE OUTPUT_VARIABLE MODULE_PATH)

    # Convert slashes to hyphens to generate a unique name for the test.
    string(REGEX REPLACE "/" "-" MODULE_TEST_NAME ${MODULE_PATH})

    # Create a new executable binary for each module test. Don't add the source file if it doesn't exist.
    set(SOURCE_FILE_IF_IT_EXISTS "")
    if(EXISTS ${PROJECT_SOURCE_DIR}/src/${SOURCE_FILE})
        set(SOURCE_FILE_IF_IT_EXISTS src/${SOURCE_FILE})
    endif()
    add_executable(${MODULE_TEST_NAME} ${ABSOLUTE_TEST_DIRECTORY_PATH}/${TEST_FILE} ${SOURCE_FILE_IF_IT_EXISTS})

    # Include files from the include directory.
    target_include_directories(${MODULE_TEST_NAME} BEFORE PRIVATE
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/src
    )

    # Link in the Unity and Modbus libraries
    target_link_libraries(${MODULE_TEST_NAME} unity ${PROJECT_SHORTNAME})

    # Include the Unity headers.
    target_include_directories(${MODULE_TEST_NAME} PRIVATE ${unity_SOURCE_DIR}/src)


    # if(NOT BUILD_SHARED_LIBS AND MSVC)
    #   target_compile_options(${target} PRIVATE /MT)
    #   target_link_options(${target} PRIVATE /INCREMENTAL:NO /NODEFAULTLIB:MSVCRT)
    # endif()
endforeach ()

add_test(NAME version COMMAND version)
message(STATUS "Added test: version")

if(WIN32)
    set(SERVER_TEST unit-server.exe)
    set(CLIENT_TEST unit-client.exe)
    # TODO: Fix the client-server-test.cmd.in file in Windows environment
    # configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/client-server-test.cmd.in 
    #                 ${CMAKE_BINARY_DIR}/unit-tests.cmd @ONLY)
    # add_test(NAME unit-tests
    #     COMMAND sh ${CMAKE_BINARY_DIR}/unit-tests.cmd)
    # message(STATUS "Added test: unit-tests command script")
else()
    set(SERVER_TEST unit-server)
    set(CLIENT_TEST unit-client)
    configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/client-server-test.sh.in 
                    ${CMAKE_BINARY_DIR}/unit-tests.sh @ONLY)
    add_test(NAME unit-tests
        COMMAND sh ${CMAKE_BINARY_DIR}/unit-tests.sh)
    message(STATUS "Added test: unit-tests shell script")
endif()
