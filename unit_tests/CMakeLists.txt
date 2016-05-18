set(NAME unit_tests)

set(SOURCES
  semihosting.cpp
  mem.c  
  test.cpp
  Logger/LoggerTest.cpp
  Comm/CommTest.cpp   
)

add_executable(${NAME} EXCLUDE_FROM_ALL ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_LIST_DIR}/linker.ld")

target_link_libraries(${NAME} platform emlib gtest gmock logger)

target_require_semihosting(${NAME})

get_target_property(existing_link_flags ${NAME} LINK_FLAGS)
set_target_properties(${NAME} PROPERTIES LINK_FLAGS "${existing_link_flags} -u _printf_float")

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})

add_custom_target(unit_tests.run
  COMMAND ${QEMU} -verbose -verbose -board generic -mcu ${QEMU_MCU} -nographic -monitor null -image ${EXEC_OBJ} -semihosting-config "arg=tests,arg=--gtest_output=xml:${OUTPUT_PATH}/unit-tests.xml"

  DEPENDS ${NAME}
)
