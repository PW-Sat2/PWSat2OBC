set(NAME mission)

set(SOURCES
    mission.c    
    antenna.c
    sail.c
    include/mission/mission.h
    include/mission/antenna.h
    include/mission/sail.h
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)

target_format_sources(${NAME} "${SOURCES}")