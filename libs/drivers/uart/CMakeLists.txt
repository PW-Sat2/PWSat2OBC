set(NAME uart)

set(SOURCES
    uart.cpp
    efm.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} freeRTOS)

target_link_libraries(${NAME} base)

target_link_libraries(${NAME} platform)

target_link_libraries(${NAME} emlib)
target_link_libraries(${NAME} efm_support)

target_link_libraries(${NAME} logger)
target_link_libraries(${NAME} emdrv)
target_link_libraries(${NAME} gsl)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/uart)

target_format_sources(${NAME} "${SOURCES}")
