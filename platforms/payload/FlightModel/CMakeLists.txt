set(SOURCES
    
)

add_library(pld_platform INTERFACE ${SOURCES})

target_link_libraries(platform INTERFACE base)

target_include_directories (platform INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Include)
