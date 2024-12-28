# src directory CMakeLists.txt for libmodbus
# Copyright © Pascal JEAN <epsilonrt@gmail.com>
# SPDX-License-Identifier: BSD-3-Clause

# Libraries 
file(GLOB_RECURSE src_modbus  ${LIBMODBUS_SRC_DIR}/*.c)

add_library(${PROJECT_SHORTNAME} ${src_modbus})
set_target_properties(${PROJECT_SHORTNAME} PROPERTIES 
  OUTPUT_NAME ${PROJECT_SHORTNAME} 
  CLEAN_DIRECT_OUTPUT 1 
  VERSION ${PROJECT_VERSION} 
  SOVERSION ${PROJECT_VERSION_MAJOR}
  )
target_include_directories(${PROJECT_SHORTNAME} BEFORE PRIVATE ${CMAKE_BINARY_DIR})

if(WIN32)
  find_library(WSOCK32_LIBRARY wsock32)
  find_library(WS2_32_LIBRARY ws2_32)
  if (WSOCK32_LIBRARY AND WS2_32_LIBRARY)
    message(STATUS "Found wsock32 and ws2_32 libraries")
    target_link_libraries(${PROJECT_SHORTNAME} ${WSOCK32_LIBRARY} ${WS2_32_LIBRARY})
  endif()
  target_compile_definitions(${PROJECT_SHORTNAME} PRIVATE HAVE_CONFIG_H)
  target_compile_definitions(${PROJECT_SHORTNAME} PRIVATE DLLBUILD)
  target_compile_definitions(${PROJECT_SHORTNAME} PRIVATE _CRT_SECURE_NO_DEPRECATE=1)
  target_compile_definitions(${PROJECT_SHORTNAME} PRIVATE _CRT_NONSTDC_NO_DEPRECATE=1)
  target_compile_definitions(${PROJECT_SHORTNAME} PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS)
  target_compile_definitions(${PROJECT_SHORTNAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
  target_sources(${PROJECT_SHORTNAME} PRIVATE ${CMAKE_BINARY_DIR}/modbus.rc)
endif(WIN32)


install (TARGETS ${PROJECT_SHORTNAME}
  # IMPORTANT: Add the ${PROJECT_SHORTNAME} library to the "export-set"
  EXPORT ${PROJECT_SHORTNAME}
  # LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  )
