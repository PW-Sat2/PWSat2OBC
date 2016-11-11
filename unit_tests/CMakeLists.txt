set(NAME unit_tests)

set(SOURCES
  semihosting.cpp
  mem.cpp
  heap.c
  test.cpp
  utils.cpp
  Logger/LoggerTest.cpp
  FileSystem/FileSystemTest.cpp
  FileSystem/YaffsOSGlue.c
  FileSystem/MemoryDriver.cpp
  FileSystem/NANDGeometryTest.cpp
  FileSystem/MemoryDriver.cpp
  FileSystem/EccTest.cpp
  Comm/CommTest.cpp
  base/ReaderTest.cpp
  base/WriterTest.cpp
  MissionPlan/MissionPlanTest.cpp  
  MissionPlan/DeployAntennaActionTest.cpp
  MissionPlan/MissionTestHelpers.cpp
  MissionPlan/SailSystemStateTest.cpp
  MissionPlan/ActionDispatchTest.cpp
  I2C/FallbackI2CBusTest.cpp
  I2C/ErrorHandlingI2CBusTest.cpp
  I2C/I2CMock.cpp
  os/os.cpp
  os/os.hpp
  OsMock.hpp
  adxrs453/adxrs453Test.cpp
  time/time.cpp
  time/timer.cpp
  time/persistance.cpp
  mock/FsMock.hpp
  mock/FsMock.cpp
  assert.cpp
)

add_executable(${NAME} ${SOURCES})

if(${ENABLE_COVERAGE})
  set(LD_COVERAGE "-fprofile-arcs")
  target_link_libraries(${NAME} gcov)
  target_compile_definitions(${NAME} PRIVATE ENABLE_COVERAGE=1)
else()
  set(LD_COVERAGE )
endif(${ENABLE_COVERAGE})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_LIST_DIR}/linker.ld -u _printf_float -specs=rdimon.specs ${LD_COVERAGE}")


target_link_libraries(${NAME}
    m
    platform
    emlib
    gtest
    gmock
    logger
    yaffs
    storage
    comm
    fs    
    base
    mission
    rapidcheck
    rapidcheck.gtest
    rapidcheck.gmock
    emdrv
    adxrs453    
    time
)

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
            "--exclude=unit_tests.*"
            "--exclude=libs.external.*"
            --html
            --html-details
            --print-summary
            --sort-percentage
            "--output=${REPORTS_PATH}/coverage/index.html"
       DEPENDS ${NAME}.run
    )
endif()
