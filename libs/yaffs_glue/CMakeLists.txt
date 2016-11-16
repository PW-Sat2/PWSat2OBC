set(NAME yaffs_glue)

add_library(${NAME} STATIC    
    yaffs_os_glue.cpp
)

target_link_libraries(${NAME} PUBLIC    
    yaffs
    logger
)
