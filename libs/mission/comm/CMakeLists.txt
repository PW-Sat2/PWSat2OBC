set(NAME mission_comm)

set(SOURCES
    Include/mission/comm_wdog.hpp
    comm_wdog.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(${NAME}
    mission
    state
    comm
    telecommunication
)

target_format_sources(${NAME} "${SOURCES}")
