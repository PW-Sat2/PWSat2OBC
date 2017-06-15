set(NAME telemetry_exp)

set(SOURCES
    Include/telemetry/collect_exp.hpp
    collect_exp.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    logger
    mission
    state
    experiments
    telemetry
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/telemetry)

target_format_sources(${NAME} "${SOURCES}")
