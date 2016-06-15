set(NAME uart)

set(SOURCES
    uart.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} freeRTOS base platform logger)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/uart)

target_format_sources(${NAME} "${SOURCES}")
