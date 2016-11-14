set(SOURCES
    platform.c
)

add_library(platform STATIC ${SOURCES})

target_link_libraries(platform PRIVATE base)

target_include_directories (platform INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Include)
