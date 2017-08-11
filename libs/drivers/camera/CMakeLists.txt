set(NAME camera)

set(SOURCES
    camera.cpp
    command_factory.cpp
    camera_low_level.cpp
    camera_types.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}        
    logger
    base
    uart
    error_counter
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/camera)
target_format_sources(${NAME} "${SOURCES}")