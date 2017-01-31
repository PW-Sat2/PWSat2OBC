set(NAME burtc)

set(SOURCES
    burtc.cpp
    Include/burtc/burtc.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
	base
	platform
    emlib
    logger    
	time    
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/burtc)

target_format_sources(${NAME} "${SOURCES}")
