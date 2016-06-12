set(NAME base)

set(SOURCES
    reader.c
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/base)
target_format_sources(${NAME} "${SOURCES}")
