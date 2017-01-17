set(NAME boot)

generate_version_file(${CMAKE_CURRENT_BINARY_DIR}/gen/version.h)

set(SOURCES
    background.c
    comms.c
    crc.c
    flash.c
    main.c
    xmodem.c
)

add_executable(${NAME} ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${PLATFORM_LINKER_SCRIPT} -specs=nano.specs -specs=nosys.specs")

target_link_libraries(${NAME}
      emlib
      bspLib
      assert
)

target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen)

target_generate_hex(${NAME})

target_memory_report(${NAME})

target_asm_listing(${NAME})

target_jlink_flash(${NAME})

target_format_sources(${NAME} "${SOURCES}")

set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS "\
    -Wno-unused-parameter \
    -Wno-strict-prototypes \
    -Wno-unused-but-set-variable \
    -Wno-old-style-declaration \
")