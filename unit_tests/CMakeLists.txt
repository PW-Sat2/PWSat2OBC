set(NAME unit_tests)

set(SOURCES
  semihosting.cpp
  mem.c
  test.cpp
  utils.cpp
  Logger/LoggerTest.cpp   
  FileSystem/FileSystemTest.cpp
  FileSystem/YaffsOSGlue.c
  FileSystem/MemoryDriver.cpp
  FileSystem/NANDGeometryTest.cpp 
  FileSystem/MemoryDriver.cpp  
  Comm/CommTest.cpp
  base/ReaderTest.cpp
  base/WriterTest.cpp
  os/os.cpp
  os/os.hpp
  OsMock.hpp
)

add_executable(${NAME} ${SOURCES})

if(${ENABLE_COVERAGE})
  set(LD_COVERAGE "-fprofile-arcs")
  target_link_libraries(${NAME} gcov)
  target_compile_definitions(${NAME} PRIVATE ENABLE_COVERAGE=1)
else()
  set(LD_COVERAGE )
endif(${ENABLE_COVERAGE})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_LIST_DIR}/linker.ld ${LD_COVERAGE}")

target_link_libraries(${NAME} platform emlib gtest gmock logger yaffs storage comm)

target_require_semihosting(${NAME})

get_target_property(existing_link_flags ${NAME} LINK_FLAGS)
set_target_properties(${NAME} PROPERTIES LINK_FLAGS "${existing_link_flags} -u _printf_float")

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})

add_custom_target(unit_tests.run
  COMMAND ${QEMU} -board generic -mcu ${QEMU_MCU} -nographic -monitor null -image ${EXEC_OBJ} -semihosting-config "arg=tests,arg=--gtest_output=xml:${OUTPUT_PATH}/unit-tests.xml"

  DEPENDS ${NAME}
)

if(${ENABLE_COVERAGE})
    add_custom_target(${NAME}.coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${REPORTS_PATH}/coverage
        COMMAND ${PYTHON_EXECUTABLE}
            -c "import gcovr.driver; gcovr.driver.main_()"
            "--gcov-executable=${CMAKE_GCOV}"
            "--root=${CMAKE_SOURCE_DIR}"
            "--object-directory=${CMAKE_BINARY_DIR}"
            --html
            --html-details
            --print-summary
            --sort-percentage
            "--output=${REPORTS_PATH}/coverage/index.html"
       DEPENDS ${NAME}.run
    )
endif()
