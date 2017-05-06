set(NAME boot)

set(SOURCES
    background.c
    comms.c
    crc.c
    flash.c
    main.c
    xmodem.c
    fault_handlers.c
    boot.c
)

add_executable(${NAME} ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${LOWER_LINKER_SCRIPT} -specs=nano.specs -specs=nosys.specs")

target_link_libraries(${NAME}
      platform
      emlib
      bspLib
      assert
)

target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen)

target_generate_hex(${NAME})

target_memory_report(${NAME})

target_asm_listing(${NAME})

target_jlink_flash(${NAME} ${LOWER_ADDRESS})

target_format_sources(${NAME} "${SOURCES}")

set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS "\
    -Wno-unused-parameter \
    -Wno-strict-prototypes \
    -Wno-unused-but-set-variable \
    -Wno-old-style-declaration \
")

