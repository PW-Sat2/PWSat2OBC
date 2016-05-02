set(NAME swo)

set(SOURCES
    swo.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} platform)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_format_sources(${NAME} "${SOURCES}")
