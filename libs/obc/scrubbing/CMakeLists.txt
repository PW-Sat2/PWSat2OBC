set(NAME obc_scrubbing)

set(SOURCES
    scrubbing.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	scrubber
	obc_hardware
	program_flash
	mission_power
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/obc)
