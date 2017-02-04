set(NAME pwsat)

generate_version_file(${CMAKE_CURRENT_BINARY_DIR}/gen/version.h)

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
    commands/rtc.cpp
    obc.h
    obc.cpp
    fault_handlers.cpp
)

add_executable(${NAME} ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${PLATFORM_LINKER_SCRIPT} -specs=nano.specs -specs=nosys.specs")

target_link_libraries(${NAME}
    platform
    freeRTOS
    base
    free_rtos_wrapper
    comm
    leuart
    swo
    logger
    SwoEndpoint
    i2c
    fs
    yaffs_glue
    time
    mission
    eps
    adcs
    power
    power_eps
    terminal
    assert
    gsl
    n25q
    antenna
    exp_fibo
    mission_antenna
    mission_sail
    mission_time
    mission_experiments
    yaffs
    obc_storage
    obc_communication
    obc_hardware
    obc_experiments
    spi
    efm_emlib
    burtc
    obc_adcs
    rtc
    mission_adcs
)

target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen)

target_generate_hex(${NAME})

target_memory_report(${NAME})

target_asm_listing(${NAME})

target_jlink_flash(${NAME})

target_format_sources(${NAME} "${SOURCES}")

target_eclipse_debug_configs(${NAME} Run Attach)

add_custom_target(swo_viewer
    COMMAND ${JLINKSWO} -cpufreq 14000000 -device ${DEVICE} -itmmask 0x3
)
