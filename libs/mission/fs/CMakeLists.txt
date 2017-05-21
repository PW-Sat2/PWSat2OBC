set(NAME mission_fs)

set(SOURCES    
    Include/mission/fs.hpp
    fs.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)

target_link_libraries(${NAME}
	state
	fs
	mission   
)

target_format_sources(${NAME} "${SOURCES}")
