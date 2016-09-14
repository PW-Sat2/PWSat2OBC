set(NAME i2c)

set(SOURCES    
    i2c.c
    fallback.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}     
    platform 
    base 
    emlib 
    logger     
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/i2c)

target_format_sources(${NAME} "${SOURCES}")
