set(NAME pwsat)

set(SOURCES
    main.c
    terminal.c
    commands/ping.c
    commands/time.c
    commands/camera.c
    commands/comm.c
    commands/file_system.c
    devices/eps.c
    openSail.c
    obc_time.c
    obc.h
    fault_handlers.c
)

add_executable(${NAME} ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${PLATFORM_LINKER_SCRIPT} -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}.map")

target_link_libraries(${NAME} 
    platform
    freeRTOS
    base
    os
    comm
    leuart
    camera
    swo
    logger
    SwoEndpoint
    i2c
    fs
)

target_generate_hex(${NAME})

target_memory_report(${NAME})

target_jlink_flash(${NAME})

target_format_sources(${NAME} "${SOURCES}")

add_custom_target(swo_viewer
    COMMAND ${JLINKSWO} -cpufreq 14000000 -device ${DEVICE} -itmmask 0x3
)
