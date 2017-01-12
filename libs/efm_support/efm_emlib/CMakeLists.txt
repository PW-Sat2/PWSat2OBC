set(NAME efm_emlib)

set(SOURCES
    emlib.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	efm_support
)

