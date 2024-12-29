# main CMakeLists.txt file for libmodbus
# Copyright © Pascal JEAN <epsilonrt@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.14)

# Appends the cmake/modules path to MAKE_MODULE_PATH variable.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" "${CMAKE_CURRENT_BINARY_DIR}" ${CMAKE_MODULE_PATH})

set(LIBMODBUS_SRC_DIR ${CMAKE_SOURCE_DIR}/src)

include(AutoconfHelper)
include(LibModbusAutoconfVersion)
GetAutoconfVersion(LIBMODBUS "${CMAKE_SOURCE_DIR}")

project(libmodbus
  VERSION ${LIBMODBUS_VERSION}
  DESCRIPTION "A Modbus library for Linux, Mac OS X, FreeBSD, QNX and Windows"
  HOMEPAGE_URL "http://libmodbus.org/"
  LANGUAGES C)
set(PROJECT_BUGREPORT "https://github.com/stephane/libmodbus/issues")

string(SUBSTRING ${CMAKE_PROJECT_NAME} 3 -1 PROJECT_SHORTNAME) # remove "lib" prefix

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

set(CMAKE_VERBOSE_MAKEFILE OFF)

# Offer the user the choice of overriding the installation directories
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for header files")
set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")

set(DEF_INSTALL_CMAKE_DIR lib)
set(DEF_INSTALL_DOC_DIR share/doc)
set(DEF_INSTALL_DATA_DIR share)

set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH "Installation directory for CMake files")
set(INSTALL_DOC_DIR ${DEF_INSTALL_DOC_DIR} CACHE PATH "Installation directory for documentation")
set(INSTALL_DATA_DIR ${DEF_INSTALL_DATA_DIR} CACHE PATH "Installation directory for data")

include(CMakePrintHelpers)

message(STATUS "Configuring ${PROJECT_SHORTNAME} ${PROJECT_VERSION}....")
message(STATUS "> ${PROJECT_DESCRIPTION}")
message(STATUS ">  Building for ${CMAKE_SYSTEM_PROCESSOR} architecture.")
message(STATUS ">  C compiler: ${CMAKE_C_COMPILER} (${CMAKE_C_COMPILER_ID})")
message(STATUS ">  Install prefix: ${CMAKE_INSTALL_PREFIX}")
# cmake_print_variables(LIBMODBUS_VERSION_MAJOR)
# cmake_print_variables(LIBMODBUS_VERSION_MINOR)
# cmake_print_variables(LIBMODBUS_VERSION_MICRO)
# cmake_print_variables(LIBMODBUS_VERSION)

include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)

check_function_exists(accept4 HAVE_ACCEPT4)
check_include_files(arpa/inet.h HAVE_ARPA_INET_H)
check_include_files(byteswap.h HAVE_BYTESWAP_H)

check_symbol_exists(TIOCM_RTS "sys/ioctl.h" HAVE_DECL_TIOCM_RTS)
check_symbol_exists(TIOCSRS485 "sys/ioctl.h" HAVE_DECL_TIOCSRS485)
check_symbol_exists(TIOCSRS485 "sys/ioctl.h" HAVE_DECL_TIOCSRS485)

check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(errno.h HAVE_ERRNO_H)
check_include_files(fcntl.h HAVE_FCNTL_H)
check_function_exists(getaddrinfo HAVE_GETADDRINFO)
check_function_exists(gettimeofday HAVE_GETTIMEOFDAY)
check_function_exists(inet_pton HAVE_INET_PTON)
check_function_exists(inet_ntop HAVE_INET_NTOP)

check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(limits.h HAVE_LIMITS_H)
check_include_files(linux/serial.h HAVE_LINUX_SERIAL_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(netdb.h HAVE_NETDB_H)
check_include_files(netinet/in.h HAVE_NETINET_IN_H)
check_include_files(netinet/tcp.h HAVE_NETINET_TCP_H)

check_function_exists(select HAVE_SELECT)
check_function_exists(socket HAVE_SOCKET)

check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_function_exists(strerror HAVE_STRERROR)

check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(string.h HAVE_STRING_H)
check_function_exists(strlcpy HAVE_STRLCPY)

check_include_files(sys/ioctl.h HAVE_SYS_IOCTL_H)
check_include_files(sys/params.h HAVE_SYS_PARAMS_H)
check_include_files(sys/socket.h HAVE_SYS_SOCKET_H)
check_include_files(sys/stat.h HAVE_SYS_STAT_H)
check_include_files(sys/time.h HAVE_SYS_TIME_H)
check_include_files(sys/types.h HAVE_SYS_TYPES_H)
check_include_files(termios.h HAVE_TERMIOS_H)
check_include_files(time.h HAVE_TIME_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files(winsock2.h HAVE_WINSOCK2_H)

check_function_exists(fork HAVE_FORK)
check_function_exists(vfork HAVE_VFORK)

set(WINVER 0x0501)

AC_USE_SYSTEM_EXTENSIONS()
# message(STATUS "  C required definitions: ${CMAKE_REQUIRED_DEFINITIONS}")

AC_C_INLINE()
AC_C_CONST()
AC_SYS_LARGEFILE()
AC_HEADER_STDC()
AC_TYPE_SIZE_T()
AC_TYPE_SSIZE_T()
AC_TYPE_INT64_T()
AC_TYPE_UINT16_T()
AC_TYPE_UINT32_T()
AC_TYPE_UINT8_T()

if(NOT MSVC)
  add_compile_options(-Wall -Wmissing-declarations -Wmissing-prototypes -Wnested-externs -Wpointer-arith -Wpointer-arith -Wsign-compare -Wchar-subscripts -Wstrict-prototypes -Wshadow -Wformat-security)
endif()

configure_file( ${LIBMODBUS_SRC_DIR}/modbus-version.h.in
                ${CMAKE_BINARY_DIR}/modbus-version.h @ONLY)

configure_file( ${CMAKE_SOURCE_DIR}/cmake/config.h.in 
                ${CMAKE_BINARY_DIR}/config.h)

if(WIN32)
  configure_file( ${CMAKE_SOURCE_DIR}/cmake/modbus.rc.in
                  ${CMAKE_BINARY_DIR}/modbus.rc @ONLY)
  configure_file( ${LIBMODBUS_SRC_DIR}/win32/modbus.dll.manifest.in
                  ${CMAKE_BINARY_DIR}/modbus.dll.manifest @ONLY)
endif()

# Make relative paths absolute (needed later on)
foreach(p LIB INCLUDE CMAKE DATA DOC EXAMPLES)
  set(var INSTALL_${p}_DIR)
  if(NOT IS_ABSOLUTE "${${var}}")
    set(${var} "${CMAKE_INSTALL_PREFIX}/${${var}}")
  endif()
endforeach()

set (MODBUS_SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set (MODBUS_INC_DIR ${PROJECT_SOURCE_DIR}/include)
set (MODBUS_LIB_DIR ${PROJECT_BINARY_DIR}/lib)
set (MODBUS_INSTALL_CMAKE_DIR "${INSTALL_CMAKE_DIR}/${PROJECT_SHORTNAME}/cmake")
set (MODBUS_INSTALL_DOC_DIR "${INSTALL_DOC_DIR}/${PROJECT_SHORTNAME}")
set (MODBUS_INSTALL_EXAMPLES_DIR "${MODBUS_INSTALL_DOC_DIR}/examples")
set (MODBUS_INSTALL_DATA_DIR "${INSTALL_DATA_DIR}/${PROJECT_SHORTNAME}")

configure_file( ${CMAKE_SOURCE_DIR}/cmake/libmodbus.pc.in 
                ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)

# Add the library source directory
add_subdirectory(src)

# CTest configuration
set(TEST_DIRECTORY ctest)
set(TEST_FILE_PREFIX test_)

include(CTest)
enable_testing()
add_subdirectory(ctest)

# Packaging configuration
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
