set(NAME eps)

set(SOURCES
    eps.cpp
    hk.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    i2c    
    logger
    base
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/eps)
target_format_sources(${NAME} "${SOURCES}")