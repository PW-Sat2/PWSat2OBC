set(NAME telemetry_fdir)

set(SOURCES
    Include/telemetry/collect_fdir.hpp
    collect_fdir.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    logger
    mission
    state
    gyro
    telemetry
    obc_fdir
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/telemetry)

target_format_sources(${NAME} "${SOURCES}")
