set(NAME comm)

set(SOURCES
    comm.cpp
    Frame.cpp
    Include/comm/Beacon.hpp
    Include/comm/comm.hpp
    Include/comm/CommDriver.hpp
    Include/comm/Frame.hpp
    Include/comm/IHandleFrame.hpp
    Include/comm/ITransmitFrame.hpp
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
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/comm)

target_format_sources(${NAME} "${SOURCES}")
