set(NAME safe_mode)

generate_version_file(${CMAKE_CURRENT_BINARY_DIR}/gen/version.h)

set(SOURCES
    steps/scrub_bootloader/step.cpp
    steps/revert_boot_slots/step.cpp
    steps/scrub_program/step.cpp
    steps/erase_n25q/step.cpp
    
    main.cpp   
    safe_mode.cpp
    error_counter.cpp
    dummy_os.cpp
    fault_handler.cpp
)

add_executable(${NAME} ${SOURCES})

target_compile_options(${NAME} PRIVATE "-Os")

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${SAFE_MODE_LINKER_SCRIPT} -L ${MCU_PLATFORM_PATH} -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections")

target_link_libraries(${NAME}
    platform
    gsl
    emlib
    boot_params
    logger
    program_flash
    standalone_flash_driver
    standalone_spi
    scrubber
    msc
    n25q
    version
)

target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen)

target_generate_padded_bin(${NAME} 32768 0x00)

target_memory_report(${NAME})

target_asm_listing(${NAME})

target_format_sources(${NAME} "${SOURCES}")

target_eclipse_debug_configs(${NAME} Run Attach)
