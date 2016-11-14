set(NAME pwsat)

set(SOURCES
    main.cpp
    terminal.cpp
    commands/ping.cpp
    commands/time.cpp
    commands/comm.cpp
    commands/antenna.cpp
    commands/file_system.cpp
    commands/state_command.cpp
    commands/i2c_test_command.cpp
    obc.h
    obc.cpp
    fault_handlers.cpp
    mission.cpp
)

add_executable(${NAME} ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${PLATFORM_LINKER_SCRIPT} -specs=nano.specs -specs=nosys.specs")

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
    telecommand_handling
    gsl
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
