
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/unit-test.h.in 
                ${CMAKE_CURRENT_BINARY_DIR}/unit-test.h @ONLY)

add_executable(version version.c)
target_link_libraries(version ${PROJECT_SHORTNAME}-shared)
target_include_directories(version BEFORE PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
)
add_test(NAME version COMMAND version)

add_executable(random-test-client random-test-client.c)
target_link_libraries(random-test-client ${PROJECT_SHORTNAME}-shared)
target_include_directories(random-test-client BEFORE PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
)
add_test(NAME random-test-client COMMAND random-test-client)

add_executable(random-test-server random-test-server.c)
target_link_libraries(random-test-server ${PROJECT_SHORTNAME}-shared)
target_include_directories(random-test-server BEFORE PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
)

add_executable(unit-test-server unit-test-server.c)
target_link_libraries(unit-test-server ${PROJECT_SHORTNAME}-shared)
target_include_directories(unit-test-server BEFORE PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
)

add_executable(unit-test-client unit-test-client.c)
target_link_libraries(unit-test-client ${PROJECT_SHORTNAME}-shared)
target_include_directories(unit-test-client BEFORE PRIVATE
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
)
add_test(NAME unit-test-client COMMAND unit-test-client)


