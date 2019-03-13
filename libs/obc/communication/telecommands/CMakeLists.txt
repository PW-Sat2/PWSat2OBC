set(NAME obc_telecommands)

set(SOURCES
    ping.cpp
    boot_settings.cpp
    power.cpp
    i2c.cpp
	memory.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	telecommunication
	logger
	program_flash
	boot_settings
	power
	state
	eps
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/obc/telecommands)
