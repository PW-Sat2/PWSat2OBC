set(NAME gyro)

set(SOURCES
    driver.cpp
    GyroscopeTelemetry.cpp
    Include/gyro/gyro.h
    Include/gyro/driver.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    platform
    logger
    i2c
    gsl
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_format_sources(${NAME} "${SOURCES}")
