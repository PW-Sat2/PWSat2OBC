set(NAME scrubber)

set(SOURCES
    program.cpp
    bootloader.cpp
    boot_settings.cpp
    safe_mode.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} PUBLIC
	base
	program_flash
	msc
	fm25w
	boot_settings
	freeRTOS
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/scrubber)
