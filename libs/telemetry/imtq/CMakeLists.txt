set(NAME telemetry_imtq)

set(SOURCES
    Include/telemetry/IImtqTelemetryCollector.hpp
    Include/telemetry/ImtqTelemetryCollector.hpp
    Include/telemetry/collect_imtq.hpp
    ImtqTelemetryCollector.cpp
    collect_imtq.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    logger
    mission
    state
    imtq
    telemetry
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/telemetry)

target_format_sources(${NAME} "${SOURCES}")
