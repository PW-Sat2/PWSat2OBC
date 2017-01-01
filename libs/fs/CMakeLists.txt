set(NAME fs)

add_library(${NAME} STATIC        
    yaffs.cpp
    extension.cpp
)

target_link_libraries(${NAME} PUBLIC
    yaffs
    logger
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/fs) 