set(NAME payload)

set(SOURCES
    Include/payload/payload.h
    payload.cpp
    commands/base.cpp
    commands/RadFET.cpp
    devices.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
    platform 
    base 
    emlib 
    logger     
    gsl
    i2c
    gpio
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/${NAME})
