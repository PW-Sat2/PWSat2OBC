set(NAME unit_tests_communication)

set(SOURCES
  TeleCommandHandlingTest.cpp
  FrameContentsWriterTest.cpp
  Telecommands/DownloadFileTelecommandTest.cpp
  Telecommands/EnterIdleStateTelecommandTest.cpp
  Telecommands/RemoveFileTelecommandTest.cpp
  Telecommands/ListFilesTelecommandTest.cpp
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME}    
    telecommunication
    obc_telecommands
    unit_tests_base
)

