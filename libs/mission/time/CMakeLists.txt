set(NAME mission_time)

set(SOURCES    
    Include/mission/time.hpp
    time.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)

target_link_libraries(${NAME}
	mission 
	state
	time
)

target_format_sources(${NAME} "${SOURCES}")
