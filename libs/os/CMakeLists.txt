set(NAME os)

set(SOURCES
    os.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} PUBLIC base)

target_link_libraries(${NAME} PRIVATE freeRTOS)

target_format_sources(${NAME} "${SOURCES}")
