set(NAME pwsat)

set(SOURCES 
    main.c 
    terminal.c
    drivers/leuart.c
    drivers/swo.c
    commands/ping.c
)

add_executable(${NAME} ${SOURCES})

target_link_libraries(${NAME} platform freeRTOS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_generate_hex(${NAME})

target_jlink_flash(${NAME} "${CMAKE_CURRENT_SOURCE_DIR}/../jlink/flash.jlink")

target_format_sources(${NAME} "${SOURCES}")

add_custom_target(${NAME}.gdb
  COMMAND ${JLinkGDBServer} -device ${DEVICE}
  DEPENDS ${NAME}
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
