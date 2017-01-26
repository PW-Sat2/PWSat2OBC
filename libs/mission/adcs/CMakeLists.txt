set(NAME mission_adcs)

set(SOURCES    
    Include/mission/adcs.hpp
    adcs.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)

target_link_libraries(${NAME}
	mission 
	state
	adcs
	mission_obc
)

target_format_sources(${NAME} "${SOURCES}")
