set(NAME efm_support)

add_library(${NAME} INTERFACE)

target_link_libraries(${NAME} INTERFACE 
    emlib
    emdrv
    platform
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

add_subdirectory(efm_emlib)