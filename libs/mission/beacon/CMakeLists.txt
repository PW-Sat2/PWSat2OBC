set(NAME mission_beacon)

set(SOURCES    
    Include/mission/BeaconUpdate.hpp
    BeaconUpdate.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)

target_link_libraries(${NAME}
    mission 
    state
    base
)

target_format_sources(${NAME} "${SOURCES}")
