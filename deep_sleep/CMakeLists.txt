set(NAME deep_sleep)

generate_version_file(${CMAKE_CURRENT_BINARY_DIR}/gen/version.h)

set(SOURCES
    main.cpp
    timer.cpp
)

add_executable(${NAME} ${SOURCES})

target_compile_options(${NAME} PRIVATE "-Os")

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${UPPER_LINKER_SCRIPT} -L ${MCU_PLATFORM_PATH} -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections")

target_link_libraries(${NAME}
    platform
    gsl
    emlib
    boot_params
    logger
    program_flash
    standalone_flash_driver
    standalone_spi
    standalone_i2c
    scrubber
    msc
    n25q
    version
)

target_include_directories(${NAME} 
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include
    )

target_generate_bin(${NAME})
target_generate_hex(${NAME})

target_jlink_flash(${NAME} ${UPPER_ADDRESS})

target_memory_report(${NAME})

target_asm_listing(${NAME})
