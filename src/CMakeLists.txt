set(NAME pwsat)

set(SOURCES
    main.c
    terminal.c
    commands/ping.c
    commands/time.c
    commands/camera.c
    devices/eps.c
    openSail.c
    obc_time.c
)

add_executable(${NAME} ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${PLATFORM_LINKER_SCRIPT}")

target_link_libraries(${NAME} platform freeRTOS base leuart camera swo logger SwoEndpoint i2c)

target_generate_hex(${NAME})

target_memory_report(${NAME})

target_jlink_flash(${NAME})

target_format_sources(${NAME} "${SOURCES}")
