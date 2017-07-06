set(NAME payload)

set(SOURCES
    Include/payload/payload.h
    payload.cpp
    commands/Housekeeping.cpp
    commands/Photodiodes.cpp
    commands/SunS.cpp
    commands/Temperatures.cpp
    commands/Whoami.cpp
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
    error_counter
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/${NAME})
