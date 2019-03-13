set(NAME pwsat)

set(SOURCES
    main.cpp
    checks/checks.cpp
    obc.h
    obc.cpp
        fault_handlers.cpp
        )

add_executable(${NAME} ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${UPPER_LINKER_SCRIPT} -L ${MCU_PLATFORM_PATH} -specs=nano.specs -specs=nosys.specs")


target_link_libraries(${NAME}
    platform
    pld_platform
    freeRTOS
    base
    free_rtos_wrapper
    comm
    swo
    logger
    SwoEndpoint
    i2c
    time
    mission
    eps
    power
    power_eps
    assert
    gsl
    fm25w
    obc_fdir
    obc_communication
    obc_hardware
    obc_state
    obc_scrubbing
    spi
    efm_emlib
    burtc
    rtc
    mission_time
    mission_state
    mission_comm
    mission_watchdog
    mission_fdir
    mission_power
    mission_memory
    watchdog
    boot_params
    program_flash
    scrubber
    boot_settings
    obc_memory
)

target_generate_hex(${NAME})
target_generate_bin(${NAME})

target_memory_report(${NAME})

target_asm_listing(${NAME})

target_jlink_flash(${NAME} ${UPPER_ADDRESS})

target_format_sources(${NAME} "${SOURCES}")

target_eclipse_debug_configs(${NAME} Run Attach)

add_custom_target(swo_viewer
    COMMAND ${JLINKSWO} -cpufreq 14000000 -device ${DEVICE} -itmmask 0x3
)
