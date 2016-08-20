set(NAME storage)

set(SOURCES    
    nand.c
    nand_driver.c
    nand_calculator.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
    logger 
    platform 
    emlib 
    base
    yaffs
) 

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/storage) 