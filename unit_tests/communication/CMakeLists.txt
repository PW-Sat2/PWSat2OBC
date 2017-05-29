set(NAME unit_tests_communication)

set(SOURCES
  TeleCommandHandlingTest.cpp
  FrameContentsWriterTest.cpp
  Telecommands/DownloadFileTelecommandTest.cpp
  Telecommands/EnterIdleStateTelecommandTest.cpp
  Telecommands/RemoveFileTelecommandTest.cpp
  Telecommands/SetTimeCorrectionConfigTelecommandTest.cpp
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME}
    telecommunication
    obc_telecommands
    state
    free_rtos_wrapper
    base
    unit_tests_base
)

