set(NAME safe_mode)

generate_version_file(${CMAKE_CURRENT_BINARY_DIR}/gen/version.h)

set(SOURCES
    main.cpp   
)

add_executable(${NAME} ${SOURCES})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${SAFE_MODE_LINKER_SCRIPT} -L ${PLATFORM_PATH} -specs=nano.specs -specs=nosys.specs")

target_link_libraries(${NAME}
    platform
    base
    gsl
    emlib
    boot_params
)

target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/gen)

target_generate_bin(${NAME})

target_memory_report(${NAME})

target_asm_listing(${NAME})

target_format_sources(${NAME} "${SOURCES}")

target_eclipse_debug_configs(${NAME} Run Attach)
