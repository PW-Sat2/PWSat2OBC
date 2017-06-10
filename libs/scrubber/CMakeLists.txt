set(NAME scrubber)

set(SOURCES
    program.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} PUBLIC
	base
	program_flash
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/scrubber)
