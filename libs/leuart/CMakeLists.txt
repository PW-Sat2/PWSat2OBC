set(NAME leuart)

set(SOURCES
    leuart.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} freeRTOS base)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_format_sources(${NAME} "${SOURCES}")
