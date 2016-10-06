set(NAME power)

set(SOURCES

)

add_library(${NAME} INTERFACE ${SOURCES})

target_link_libraries(${NAME} INTERFACE
    base
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

add_subdirectory(power_eps)