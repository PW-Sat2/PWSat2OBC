set(NAME ut)

set(SOURCES
  semihosting.cpp
  test.cpp
)

add_executable(${NAME} ${SOURCES})

target_link_libraries(${NAME} ${GTEST_LIB} platform)
target_include_directories(${NAME} PRIVATE ${GTEST_INCLUDE})

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})

add_custom_target(run_tests
  COMMAND ${QEMU} -board generic -mcu ${QEMU_MCU} -nographic -monitor null -image ${EXEC_OBJ}

  DEPENDS ut
)
