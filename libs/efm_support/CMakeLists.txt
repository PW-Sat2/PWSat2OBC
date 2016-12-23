set(NAME efm_support)

add_library(${NAME} INTERFACE)

target_link_libraries(${NAME} INTERFACE 
    emlib
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
