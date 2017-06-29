set(NAME telecommunication)

set(SOURCES
    include/telecommunication/downlink.h
    include/telecommunication/uplink.h
    include/telecommunication/telecommand_handling.h
    include/telecommunication/FrameContentWriter.hpp
    include/telecommunication/beacon.hpp
    telecommand_handling.cpp
    uplink.cpp
    downlink.cpp
    FrameContentWriter.cpp
    beacon.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
    base
    comm
    logger
    telemetry
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/${NAME})
