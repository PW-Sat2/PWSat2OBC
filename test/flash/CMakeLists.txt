set(NAME flash)

set(SOURCES
    main.cpp
    fault_handlers.cpp
)

add_executable(${NAME} EXCLUDE_FROM_ALL ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${UPPER_LINKER_SCRIPT} -L ${MCU_PLATFORM_PATH} -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}.map -specs=nano.specs -specs=nosys.specs")

target_link_libraries(${NAME}
    assert
    freeRTOS
    base
    swo
    logger
    SwoEndpoint
    emdrv
    free_rtos_wrapper
    spi
    n25q
    platform
    pld_platform
    efm_emlib
)

target_generate_hex(${NAME})

target_memory_report(${NAME})

target_jlink_flash(${NAME} ${UPPER_ADDRESS})

target_format_sources(${NAME} "${SOURCES}")

target_eclipse_debug_configs(${NAME} Run Attach)