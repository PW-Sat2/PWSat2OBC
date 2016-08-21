set(NAME mission)

set(SOURCES    
    antenna.c
    sail.c        
    adcs.c
    include/mission/antenna.h
    include/mission/sail.h
    include/mission/adcs_mission.h    
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    eps
    state   
    adcs 
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)

target_format_sources(${NAME} "${SOURCES}")