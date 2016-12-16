set(NAME mission_sail)

set(SOURCES    
    Include/mission/sail.hpp
    sail.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(${NAME}
	mission 
	state
	eps
)

target_format_sources(${NAME} "${SOURCES}")
