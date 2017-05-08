set(NAME mission_telemetry)

set(SOURCES
    Include/mission/telemetry.hpp
    telemetry.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(${NAME}
    mission 
    state
    fs
)

target_format_sources(${NAME} "${SOURCES}")
