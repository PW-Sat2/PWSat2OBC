set(NAME obc_telecommands)

set(SOURCES
    ping.cpp
    file_system.cpp
    experiments.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	telecommunication
	fs
	logger
	obc_experiments
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/obc/telecommands)
