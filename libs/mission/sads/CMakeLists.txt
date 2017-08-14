set(NAME mission_sads)

set(SOURCES    
    Include/mission/sads.hpp
    sads.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(${NAME}
	mission 
	state
	power
)

target_format_sources(${NAME} "${SOURCES}")
