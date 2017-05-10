set(NAME obc_telecommands)

set(SOURCES
    comm.cpp
    ping.cpp
    file_system.cpp
    experiments.cpp
    TimeTelecommand.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	telecommunication
	mission_comm
	fs
	logger
	obc_experiments
	state
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/obc/telecommands)
