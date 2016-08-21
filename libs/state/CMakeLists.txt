set(NAME state)

set(SOURCES
    state.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    time
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/state)

target_format_sources(${NAME} "${SOURCES}")