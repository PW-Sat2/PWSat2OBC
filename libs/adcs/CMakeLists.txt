set(NAME adcs)

set(SOURCES      
    adcs.cpp
    imtq.cpp
    Include/adcs/adcs.h
    Include/adcs/imtq.h
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    logger
    imtq
    i2c
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/adcs)

target_format_sources(${NAME} "${SOURCES}")