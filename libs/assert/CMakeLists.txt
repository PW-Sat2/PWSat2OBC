set(NAME assert)

set(SOURCES
    assert.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} PUBLIC swo)

target_format_sources(${NAME} "${SOURCES}")
