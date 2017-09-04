set(NAME telemetry_os)

set(SOURCES
    Include/telemetry/collect_os.hpp
    collect_os.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    mission
    state
    telemetry
)

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_format_sources(${NAME} "${SOURCES}")
