set(NAME mission_comm)

set(SOURCES
    Include/mission/comm.hpp
    comm.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(${NAME}                                                 
    mission
    state
    comm
)

target_format_sources(${NAME} "${SOURCES}")
