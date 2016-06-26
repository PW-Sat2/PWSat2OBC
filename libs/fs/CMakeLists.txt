set(NAME fs)

add_library(${NAME} STATIC
    nand_driver.c
    yaffs_os_glue.c
    fs.c
)

target_link_libraries(${NAME} PUBLIC
    storage
    yaffs
    logger
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/fs) 