set(NAME camera)

set(SOURCES
    camera_utils.cpp
    camera_internal.cpp
    camera.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} uart)

target_link_libraries(${NAME} leuart)

target_link_libraries(${NAME} logger)

target_link_libraries(${NAME} base)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/camera)

target_format_sources(${NAME} "${SOURCES}")
