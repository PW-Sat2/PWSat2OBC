set(NAME obc_telecommands)

set(SOURCES
    comm.cpp
    ping.cpp
    file_system.cpp
    experiments.cpp
    program_upload.cpp
    time.cpp
    boot_settings.cpp
    antenna.cpp
    power.cpp
    i2c.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	telecommunication
	mission_antenna
	mission_comm
	fs
	logger
	obc_experiments
	program_flash
	boot_settings
	power
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/obc/telecommands)
