set(NAME antenna)

set(SOURCES
    miniport.cpp
    antenna.cpp
    telemetry.cpp
    Include/antenna/antenna.h
    Include/antenna/miniport.h
    Include/antenna/driver.h
    Include/antenna/telemetry.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
	base
	platform
	logger
	time
    i2c
    error_counter
	)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/antenna)

target_format_sources(${NAME} "${SOURCES}")
