set(NAME mission_state)

set(SOURCES    
    Include/mission/PersistentStateSave.hpp
    PersistentStateSave.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)

target_link_libraries(${NAME}
	mission 
	state
	obc_state
)

target_format_sources(${NAME} "${SOURCES}")
