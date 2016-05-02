set(NAME logger)

set(SOURCES
    Logger.c
)

add_library(${NAME} STATIC ${SOURCES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Include/Logger)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(${NAME} base)

target_format_sources(${NAME} "${SOURCES}")
