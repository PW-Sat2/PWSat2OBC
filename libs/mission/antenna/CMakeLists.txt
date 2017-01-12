set(NAME mission_antenna)

set(SOURCES
    antenna.cpp
    Include/mission/antenna_state.h
    Include/mission/antenna_task.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    time
    state
    antenna
    mission
    gsl
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)

target_format_sources(${NAME} "${SOURCES}")
