set(NAME mission_watchdog)


add_library(${NAME} INTERFACE)

target_link_libraries(${NAME} INTERFACE
	base
	mission
	state
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
