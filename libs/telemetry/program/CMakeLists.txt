set(NAME telemetry_program)

set(SOURCES
    Include/telemetry/collect_program.hpp
    collect_program.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    logger
    mission
    state
    boot_params
    program_flash
    telemetry
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/telemetry)

target_format_sources(${NAME} "${SOURCES}")
