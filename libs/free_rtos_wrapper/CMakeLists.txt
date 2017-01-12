set(NAME free_rtos_wrapper)

set(SOURCES
    os.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} PUBLIC 
    base
    logger
)

target_link_libraries(${NAME} PRIVATE freeRTOS)

target_format_sources(${NAME} "${SOURCES}")
