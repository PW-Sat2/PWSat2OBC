set(NAME unit_tests_communication)

set(SOURCES
  TeleCommandHandlingTest.cpp
  FrameContentsWriterTest.cpp
  Telecommands/DownloadFileTelecommandTest.cpp
  Telecommands/EnterIdleStateTelecommandTest.cpp
  Telecommands/RawI2CTelecommandTest.cpp
  Telecommands/RemoveFileTelecommandTest.cpp
  Telecommands/UploadProgramTest.cpp
  Telecommands/ListFilesTelecommandTest.cpp
  Telecommands/SetTimeCorrectionConfigTelecommandTest.cpp
  Telecommands/SetTimeTelecommandTest.cpp
  Telecommands/SetBootSlotsTelecommandTest.cpp
  Telecommands/StopAntennaDeploymentTelecommandTest.cpp
  Telecommands/PowerCycleTelecommandTest.cpp
  Telecommands/SetErrorCounterConfigTelecommandTest.cpp
  Telecommands/OpenSailTelecommandTest.cpp
  Telecommands/GetErrorCountersConfigTelecommandTest.cpp
  Telecommands/SetPeriodicMessageTelecommandTest.cpp
  Telecommands/AbortExperimentTelecommandTest.cpp
  Telecommands/PerformDetumblingExperimentTelecommandTest.cpp
  Telecommands/EraseFlashTelecommandTest.cpp
  Telecommands/GetSunSDataSetsTelecommandTest.cpp
  Telecommands/PerformSailExperimentTest.cpp
  Telecommands/PhotoCommandTest.cpp
  Telecommands/PerformSADSExperimentTest.cpp
  Telecommands/SendPeriodicMessageTelecommandTest.cpp
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME}
    experiments
    -Wl,--start-group
    base
    unit_tests_base
    program_flash 
    telecommunication
    obc_telecommands
    boot_settings
    -Wl,--end-group 
)

SET( CMAKE_LINKER_FLAGS  "${CMAKE_LINKER_FLAGS} -l$<TARGET_FILE:MyLib>" )

