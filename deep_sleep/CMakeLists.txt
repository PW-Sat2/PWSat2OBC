set(NAME deep_sleep)

option(FAST_DEEP_SLEEP "Enable fast deep sleep" OFF)

generate_version_file(${CMAKE_CURRENT_BINARY_DIR}/gen/version.h)

set(SOURCES
    eps.cpp
    fault_handler.cpp
    main.cpp
    state.cpp
    flash_eraser.cpp
    timer.cpp
    scrubbing.cpp
    include/eps.hpp
    include/state.hpp
    include/flash_eraser.hpp
    include/config.hpp
    comm.cpp
    sleep.cpp
    include/timer.h
    include/comm.hpp
    include/scrubbing.hpp
)

add_executable(${NAME} ${SOURCES})

target_compile_options(${NAME} PRIVATE "-Os")

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${UPPER_LINKER_SCRIPT} -L ${MCU_PLATFORM_PATH} -specs=nano.specs -specs=nosys.specs -Wl,--gc-sections")

target_link_libraries(${NAME}
    platform
    gsl
    emlib
    boot_params
    boot_settings
    logger
    program_flash
    standalone_flash_driver
    standalone_spi
    standalone_i2c
    scrubber
    msc
    n25q
    fm25w
    version
    error_counter
    comm
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

if(FAST_DEEP_SLEEP)
    target_compile_definitions(${NAME} PUBLIC -DFAST_DEEP_SLEEP)
endif()
