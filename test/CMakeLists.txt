set(NAME ut)

set(SOURCES
  semihosting.cpp
  test.cpp
)

add_executable(${NAME} EXCLUDE_FROM_ALL ${SOURCES})

target_link_libraries(${NAME} platform gtest)

target_require_semihosting(${NAME})

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})

add_custom_target(run_tests
  COMMAND ${QEMU} -board generic -mcu ${QEMU_MCU} -nographic -monitor null -image ${EXEC_OBJ}

  DEPENDS ut
)
