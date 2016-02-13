set(NAME ut)

set(SOURCES
  semihosting.cpp
  test.cpp
)

add_executable(${NAME} EXCLUDE_FROM_ALL ${SOURCES})

target_link_libraries(${NAME} platform gtest)

target_require_semihosting(${NAME})

get_target_property(existing_link_flags ${NAME} LINK_FLAGS)
set_target_properties(${NAME} PROPERTIES LINK_FLAGS "${existing_link_flags} -u _printf_float")

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})

add_custom_target(run_tests
  COMMAND ${QEMU} -board generic -mcu ${QEMU_MCU} -nographic -monitor null -image ${EXEC_OBJ} -semihosting-config "arg=tests,arg=--gtest_output=xml:${OUTPUT_PATH}/unit-tests.xml"

  DEPENDS ut
)
