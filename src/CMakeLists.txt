set(NAME pwsat)

set(SOURCES 
    main.c 
    swo.c
)

add_executable(${NAME} ${SOURCES})

target_link_libraries(${NAME} platform freeRTOS)

target_generate_hex(${NAME})

target_jlink_flash(${NAME} "${CMAKE_CURRENT_SOURCE_DIR}/../jlink/flash.jlink")

add_custom_target(${NAME}.gdb
  COMMAND ${JLinkGDBServer} -device ${DEVICE}
  DEPENDS ${NAME}
  WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
