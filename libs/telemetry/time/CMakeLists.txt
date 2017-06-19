set(NAME telemetry_time)

set(SOURCES
    Include/telemetry/collect_internal_time.hpp
    Include/telemetry/collect_external_time.hpp
    collect_internal_time.cpp
    collect_external_time.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    logger
    mission
    state
    time
    rtc
    telemetry
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/telemetry)

target_format_sources(${NAME} "${SOURCES}")
