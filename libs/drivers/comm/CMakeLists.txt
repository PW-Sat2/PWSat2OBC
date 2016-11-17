set(NAME comm)

set(SOURCES
    comm.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
    base 
    platform 
    logger 
    emlib
    i2c
    gsl
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/comm)

target_format_sources(${NAME} "${SOURCES}")
