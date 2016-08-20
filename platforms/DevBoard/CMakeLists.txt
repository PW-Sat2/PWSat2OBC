set(SOURCES
    #extension.c
)

add_library(platform INTERFACE ${SOURCES})

#target_link_libraries(platform PRIVATE emlib)

target_include_directories (platform INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Include)
