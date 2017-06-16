set(NAME unit_tests_communication)

set(SOURCES
  TeleCommandHandlingTest.cpp
  FrameContentsWriterTest.cpp
  Telecommands/DownloadFileTelecommandTest.cpp
  Telecommands/EnterIdleStateTelecommandTest.cpp
  Telecommands/RemoveFileTelecommandTest.cpp
  Telecommands/UploadProgramTest.cpp
  Telecommands/ListFilesTelecommandTest.cpp
  Telecommands/SetBootSlotsTelecommandTest.cpp
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME}
    base
    program_flash 
    unit_tests_base
    telecommunication
    obc_telecommands
    boot_settings
)

