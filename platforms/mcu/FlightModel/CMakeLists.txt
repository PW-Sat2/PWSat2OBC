set(SOURCES
    variables.cpp   
)

add_library(platform STATIC ${SOURCES})

target_link_libraries(platform PUBLIC base)
target_link_libraries(platform PRIVATE emlib)

target_include_directories (platform PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Include)
