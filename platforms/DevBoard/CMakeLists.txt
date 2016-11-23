set(SOURCES
    
)

add_library(platform INTERFACE ${SOURCES})

target_include_directories (platform INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Include)
