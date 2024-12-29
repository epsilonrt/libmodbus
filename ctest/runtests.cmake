
if(WIN32)
    set(DAEMON start /b ${SERVER})
else(WIN32)
    set(DAEMON ${SERVER} &)
endif(WIN32)

# Daemonize the server
execute_process(COMMAND ${DAEMON} RESULT_VARIABLE CMD_RESULT OUTPUT_FILE ${CMAKE_BINARY_DIR}/${SERVER}.log)
if(CMD_RESULT)
   message(FATAL_ERROR "Error running ${SERVER}")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E 2) # Wait for the server to start up 2 seconds

# Run the client
execute_process(COMMAND ${DAEMON} RESULT_VARIABLE CMD_RESULT)
if(CMD_RESULT)
   message(FATAL_ERROR "Error running ${CLIENT}")
endif()
