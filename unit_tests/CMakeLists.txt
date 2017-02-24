set(NAME unit_tests)

set(SOURCES
  semihosting.cpp
  mem.cpp
  heap.c
  test.cpp
  utils.cpp
  os/os.cpp
  os/os.hpp
  OsMock.hpp
  OsMock.cpp
  OsMock.hpp
  assert.cpp
  I2C/I2CMock.cpp
  mock/FsMock.hpp
  mock/FsMock.cpp
  mock/emlib.cpp
  mock/AntennaMock.cpp
  mock/AntennaMock.hpp
  mock/AdcsMocks.hpp
  mock/ActionDescriptorMock.hpp
  mock/UpdateDescriptorMock.hpp
  mock/VerifyDescriprorMock.hpp
  SPI/SPIMock.cpp
  antenna/MiniportMock.cpp
  antenna/MiniportMock.hpp
  imtq/imtqTest.cpp
  imtq/imtqHighLevelTest.cpp
  Logger/LoggerTest.cpp
  FileSystem/FileSystemTest.cpp
  FileSystem/YaffsOSGlue.cpp
  FileSystem/MemoryDriver.cpp
  FileSystem/NANDGeometryTest.cpp
  FileSystem/MemoryDriver.cpp
  FileSystem/EccTest.cpp
  FileSystem/FileTest.cpp
  Comm/CommTest.cpp
  Comm/CommFrameTest.cpp
  Comm/DownlinkFrameTest.cpp
  Comm/TeleCommandHandlingTest.cpp
  Comm/UplinkFrameDecoderTest.cpp
  Comm/Telecommands/DownloadFileTelecommandTest.cpp
  base/ReaderTest.cpp
  base/WriterTest.cpp
  base/RapidTest.cpp
  base/OnLeaveTest.cpp
  MissionPlan/MissionPlanTest.cpp
  MissionPlan/MissionTestHelpers.cpp
  MissionPlan/SailSystemStateTest.cpp
  MissionPlan/TimeTaskTest.cpp
  MissionPlan/MissionLoopTest.cpp
  MissionPlan/antenna/DeployAntennasTest.cpp
  MissionPlan/antenna/AntennaMissionStateTest.cpp
  MissionPlan/beacon/BeaconUpdateTest.cpp
  MissionPlan/CompositeActionTest.cpp
  MissionPlan/adcs/AdcsPrimaryTaskTest.cpp
  MissionPlan/obc/ObcTest.cpp
  I2C/FallbackI2CBusTest.cpp
  I2C/ErrorHandlingI2CBusTest.cpp
  N25Q/N25QTest.cpp
  os/TimeoutTest.cpp
  os/EventGroupTest.cpp
  time/time.cpp
  time/timer.cpp
  time/persistance.cpp
  time/SmartWait.cpp
  time/rtc.cpp
  mock/RtcMock.hpp
  mock/comm.cpp
  mock/comm.hpp
  assert.cpp
  antenna/miniport.cpp
  antenna/antenna.cpp
  SPI/SPIDriverTest.cpp
  eigen/eigenTest.cpp
  adcs/AdcsCoordinatorTest.cpp
  Experiments/Fibo/FiboCalculatorTest.cpp
  Experiments/ExperimentTest.cpp
  Experiments/ADCS/DetumblingExperimentTest.cpp
  ErrorCounterTest.cpp
  FM25W/FM25WDriverTest.cpp
  telecommunication/FrameContentsWriterTest.cpp
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

target_memory_report(${NAME})

target_asm_listing(${NAME})

target_compile_options(${NAME} PRIVATE "-fexceptions")

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
    telecommunication
    fs    
    base
    mission
    rapidcheck
    rapidcheck_gtest
    rapidcheck_gmock
    emdrv
    time
    gsl
    antenna
    n25q
    mission_antenna
    imtq
    mission_sail
    mission_time
    eigen
    mission_experiments
    exp_fibo
    adcs
    mission_adcs
    obc_telecommands
    rtc
    error_counter
    fm25w
    mission_beacon
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

target_eclipse_debug_configs(${NAME} QEmu)

