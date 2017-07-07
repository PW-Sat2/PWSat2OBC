set(NAME leuart)

set(SOURCES
    leuart.cpp
)

add_library(${NAME} STATIC EXCLUDE_FROM_ALL ${SOURCES})

target_link_libraries(${NAME} 
    freeRTOS 
    base 
    emlib 
    platform 
    emdrv
    gsl
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/leuart)

target_format_sources(${NAME} "${SOURCES}")
