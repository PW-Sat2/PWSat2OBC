set(NAME mission)

set(SOURCES    
    antenna.cpp
    sail.cpp        
    adcs.cpp
    Include/mission/mission.h
    Include/mission/antenna.h
    Include/mission/sail.h
    Include/mission/adcs_mission.h
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    eps
    time
    state      
    adcs 
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)

target_format_sources(${NAME} "${SOURCES}")
