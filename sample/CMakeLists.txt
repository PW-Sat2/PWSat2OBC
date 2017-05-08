set(NAME sample)


set(SOURCES
    background.c
    comms.c
    main.c
    tests.c
)

add_executable(${NAME} EXCLUDE_FROM_ALL ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${UPPER_LINKER_SCRIPT} -specs=nano.specs -specs=nosys.specs")

target_link_libraries(${NAME}
    platform
    emlib
    bspLib
    assert
)


target_generate_bin(${NAME})

target_memory_report(${NAME})

target_asm_listing(${NAME})

target_jlink_flash(${NAME} ${UPPER_ADDRESS})

target_format_sources(${NAME} "${SOURCES}")

set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS "\
    -Wno-unused-parameter \
    -Wno-strict-prototypes \
    -Wno-unused-but-set-variable \
    -Wno-old-style-declaration \
    -Wno-unused-variable \
    -Wno-format \
    -Wno-type-limits \
")

target_compile_definitions(${NAME} PRIVATE -DRelease)
