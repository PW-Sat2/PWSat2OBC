set(NAME pwsat)

set(SOURCES
    main.c
    terminal.c
    commands/ping.c
    commands/time.c
    commands/comm.c
    commands/file_system.c
    commands/state_command.c
    commands/i2c_test_command.c
    commands/antenna.c
    obc.h
    fault_handlers.c
    mission.c
)

add_executable(${NAME} ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${PLATFORM_LINKER_SCRIPT} -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}.map -specs=nano.specs -specs=nosys.specs")

target_link_libraries(${NAME}
    platform
    freeRTOS
    base
    os
    comm
    leuart
    swo
    logger
    SwoEndpoint
    i2c
    fs
    yaffs_glue
    adxrs453
    time
    mission
    eps
    adcs
    power
    power_eps
    terminal
    assert
    antenna
)

target_generate_hex(${NAME})

target_memory_report(${NAME})

target_asm_listing(${NAME})

target_jlink_flash(${NAME})

target_format_sources(${NAME} "${SOURCES}")

add_custom_target(swo_viewer
    COMMAND ${JLINKSWO} -cpufreq 14000000 -device ${DEVICE} -itmmask 0x3
)
