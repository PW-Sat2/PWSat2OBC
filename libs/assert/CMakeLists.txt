set(NAME assert)

set(SOURCES
    assert.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} PUBLIC swo)

target_format_sources(${NAME} "${SOURCES}")
