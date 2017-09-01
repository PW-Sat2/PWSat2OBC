set(NAME pwsat)

set(SOURCES
    main.cpp
    terminal.cpp
    commands/ping.cpp
    commands/time.cpp
    commands/comm.cpp
    commands/antenna.cpp
    commands/file_system.cpp
    commands/i2c_test_command.cpp
    commands/heap_info.cpp
    commands/rtos_status.cpp
    commands/heap.cpp
    commands/compile_info.cpp
    commands/mission.cpp
    commands/dma.cpp
    commands/imtq.cpp
    commands/suns.cpp
    commands/rtc.cpp
    commands/fram.cpp
    commands/gyro.cpp
    commands/external_flash.cpp
    commands/obc_state.cpp
    commands/temp.cpp
    commands/eps.cpp
    commands/watchdog.cpp
    commands/state.cpp
    commands/error_counters.cpp
    commands/scrub.cpp
    commands/boot_settings.cpp
    commands/payload.cpp
    commands/memory.cpp
    commands/runlevel.cpp
    commands/sail.cpp
    commands/leop.cpp
    commands/exp_suns.cpp
    commands/camera.cpp
    commands/adcs.cpp
    checks/error_counters.cpp
    checks/checks.cpp
    obc.h
    obc.cpp
    obc_access.cpp
    fault_handlers.cpp
    blink.cpp
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
    uart
    swo
    logger
    SwoEndpoint
    i2c
    fs
    yaffs_glue
    time
    mission
    eps
    imtq
    suns
    power
    power_eps
    terminal
    assert
    gsl
    fm25w
    n25q
    antenna
    exp_fibo
    yaffs
    obc_fdir
    obc_storage
    obc_communication
    obc_hardware
    obc_experiments
    obc_state
    obc_scrubbing
    obc_camera
    spi
    efm_emlib
    burtc
    obc_adcs
    rtc
    mission_antenna
    mission_sail
    mission_time
    mission_experiments
    mission_state
    mission_adcs
    mission_leop
    mission_beacon
    mission_telemetry
    mission_comm
    mission_fs
    mission_watchdog
    mission_fdir
    mission_sads
    watchdog
    boot_params
    telemetry
    telemetry_comm
    telemetry_gyro
    telemetry_fdir
    telemetry_eps
    telemetry_exp
    telemetry_temp
    telemetry_ant
    telemetry_gpio
    telemetry_fs
    telemetry_time
    telemetry_program
    telemetry_flash_scrubbing
    telemetry_ram_scrubbing
    telemetry_imtq
    program_flash
    scrubber
    boot_settings
    obc_memory
    camera
    version
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
