# Libraries 
file(GLOB_RECURSE src_modbus  ${LIBMODBUS_SRC_DIR}/*.c)

add_library(objlib OBJECT ${src_modbus})
#UseSystemExtensions(objlib)
target_include_directories(objlib BEFORE PRIVATE
  ${CMAKE_BINARY_DIR}
)

# shared libraries need PIC
set_property(TARGET objlib PROPERTY POSITION_INDEPENDENT_CODE 1)

add_library(${PROJECT_SHORTNAME}-shared SHARED $<TARGET_OBJECTS:objlib>)
set_target_properties(${PROJECT_SHORTNAME}-shared PROPERTIES 
  OUTPUT_NAME ${PROJECT_SHORTNAME} 
  CLEAN_DIRECT_OUTPUT 1 
  VERSION ${PROJECT_VERSION} 
  SOVERSION ${PROJECT_VERSION_MAJOR}
  )
if(WIN32)
  find_library(WSOCK32_LIBRARY wsock32)
  find_library(WS2_32_LIBRARY ws2_32)
  if (WSOCK32_LIBRARY AND WS2_32_LIBRARY)
    message(STATUS "Found wsock32 and ws2_32 libraries")
    target_link_libraries(${PROJECT_SHORTNAME}-shared ${WSOCK32_LIBRARY} ${WS2_32_LIBRARY})
  endif()
  target_compile_definitions(${PROJECT_SHORTNAME}-shared PRIVATE HAVE_CONFIG_H)
  target_compile_definitions(${PROJECT_SHORTNAME}-shared PRIVATE DLLBUILD)
  target_compile_definitions(${PROJECT_SHORTNAME}-shared PRIVATE _CRT_SECURE_NO_DEPRECATE=1)
  target_compile_definitions(${PROJECT_SHORTNAME}-shared PRIVATE _CRT_NONSTDC_NO_DEPRECATE=1)
  target_compile_definitions(${PROJECT_SHORTNAME}-shared PRIVATE _WINSOCK_DEPRECATED_NO_WARNINGS)
  target_compile_definitions(${PROJECT_SHORTNAME}-shared PRIVATE _CRT_SECURE_NO_WARNINGS)
endif(WIN32)

add_library(${PROJECT_SHORTNAME}-static STATIC $<TARGET_OBJECTS:objlib>)
set_target_properties(${PROJECT_SHORTNAME}-static PROPERTIES
  OUTPUT_NAME ${PROJECT_SHORTNAME} 
  CLEAN_DIRECT_OUTPUT 1 
  VERSION ${PROJECT_VERSION} 
)

install (TARGETS ${PROJECT_SHORTNAME}-shared
  # IMPORTANT: Add the ${PROJECT_SHORTNAME} library to the "export-set"
  EXPORT ${PROJECT_SHORTNAME}
  # LIBRARY DESTINATION "${INSTALL_LIB_DIR}" COMPONENT lib
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  )

install (TARGETS ${PROJECT_SHORTNAME}-static 
  EXPORT ${PROJECT_SHORTNAME}
  # ARCHIVE DESTINATION "${INSTALL_LIB_DIR}" COMPONENT dev
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
)
