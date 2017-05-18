set(NAME scrubber)

add_library(${NAME} INTERFACE)

target_link_libraries(${NAME} INTERFACE
	base
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
