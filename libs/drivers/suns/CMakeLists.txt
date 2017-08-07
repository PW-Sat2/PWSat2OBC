set(NAME suns)

set(SOURCES
    suns.cpp
    Include/suns/suns.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    platform
    logger
    i2c
    gpio
    gsl
    error_counter
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/suns)

target_format_sources(${NAME} "${SOURCES}")
