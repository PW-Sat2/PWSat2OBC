set(NAME generate_exp_data)

set(SOURCES
    main.cpp
)

add_executable(${NAME} ${SOURCES})

target_link_libraries(${NAME} 
	base
	platform
	emlib
	fs
)

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/unit_tests/base/linker.ld -u _printf_float -specs=rdimon.specs")

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
add_custom_target(${NAME}.run
  COMMAND ${QEMU} -board generic -mcu ${QEMU_MCU} -nographic -monitor null -image ${EXEC_OBJ} -semihosting-config "arg=tests"

  DEPENDS ${NAME}
)    
