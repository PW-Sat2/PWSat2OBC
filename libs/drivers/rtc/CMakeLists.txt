set(NAME rtc)

set(SOURCES
    rtc.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
	base
	platform
	logger 
	i2c
	error_counter
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/rtc)

target_format_sources(${NAME} "${SOURCES}")
