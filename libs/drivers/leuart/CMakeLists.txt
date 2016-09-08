set(NAME leuart)

set(SOURCES
    leuart.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} freeRTOS base emlib platform emdrv)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/leuart)

target_format_sources(${NAME} "${SOURCES}")
