set(NAME time)

set(SOURCES
    time.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} base)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/time)
target_format_sources(${NAME} "${SOURCES}")
