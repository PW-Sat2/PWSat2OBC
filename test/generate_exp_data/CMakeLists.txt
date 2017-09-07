set(NAME generate_exp_data)

set(SOURCES
    main.cpp
    os_stubs.cpp
    suns_data.cpp
    sail_data.cpp
    detumbling_data.cpp
    camera_data.cpp
    payload_data.cpp
    leop_data.cpp
    radfet_data.cpp
)

add_executable(${NAME} ${SOURCES})

target_link_libraries(${NAME} 
	base
	platform
	emlib
	fs
	exp_suns
	exp_adcs
)

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${CMAKE_SOURCE_DIR}/unit_tests/base/linker.ld -u _printf_float -specs=rdimon.specs")

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/experiments)

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
add_custom_target(${NAME}.run  
  COMMAND ${QEMU} -board generic -mcu ${QEMU_MCU} -nographic -monitor null -image ${EXEC_OBJ} -semihosting-config "arg=tests"
  WORKING_DIRECTORY  ${CMAKE_CURRENT_BINARY_DIR}/experiments
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/experiments ${REPORTS_PATH}/experiments/
  DEPENDS ${NAME}
)    
