set(NAME generate_persistent_state)

set(SOURCES
    main.cpp
    os_stubs.cpp
)

add_executable(${NAME} ${SOURCES})

target_link_libraries(${NAME} 
	base
	platform
	emlib
	state
)

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/unit_tests/base/linker.ld -u _printf_float -specs=rdimon.specs")

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
add_custom_target(${NAME}.run
  COMMAND ${QEMU} -board generic -mcu ${QEMU_MCU} -nographic -monitor null -image ${EXEC_OBJ} -semihosting-config "arg=tests"
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/persistent_state ${REPORTS_PATH}/persistent_state
  DEPENDS ${NAME}
)    
