set(NAME terminal)

set(SOURCES
    terminal.cpp
    include/terminal/terminal.h
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
	base
	logger
	leuart
	gsl
	)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/terminal)
target_format_sources(${NAME} "${SOURCES}")
