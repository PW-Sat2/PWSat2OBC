set(NAME swo)

set(SOURCES
    swo.cpp
    extension.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} emlib)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/swo)

target_format_sources(${NAME} "${SOURCES}")
