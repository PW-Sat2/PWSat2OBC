set(NAME SwoEndpoint)

set(SOURCES
    SwoEndpoint.cpp
)

add_library(${NAME} STATIC ${SOURCES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Include/SwoEndpoint)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(${NAME} base logger swo freeRTOS)

target_format_sources(${NAME} "${SOURCES}")
