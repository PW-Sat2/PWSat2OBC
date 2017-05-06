set(NAME flash)

set(SOURCES
    main.cpp
)

add_executable(${NAME} EXCLUDE_FROM_ALL ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${UPPER_LINKER_SCRIPT} -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}.map -specs=nano.specs -specs=nosys.specs")

target_link_libraries(${NAME}
    assert
    platform
    freeRTOS
    base
    swo
    leuart
    logger
    SwoEndpoint
    free_rtos_wrapper
)

target_generate_hex(${NAME})

target_memory_report(${NAME})

target_jlink_flash(${NAME} ${LOWER_ADDRESS})

target_format_sources(${NAME} "${SOURCES}")
