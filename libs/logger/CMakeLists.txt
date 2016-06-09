set(NAME logger)

set(SOURCES
    Logger.c
)

add_library(${NAME} STATIC ${SOURCES})

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/logger)

target_link_libraries(${NAME} base)

target_format_sources(${NAME} "${SOURCES}")

add_subdirectory(SwoEndpoint)
