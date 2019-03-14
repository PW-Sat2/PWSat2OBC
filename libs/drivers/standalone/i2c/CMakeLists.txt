set(NAME standalone_i2c)

add_library(${NAME} STATIC
    i2c.cpp
    Include/standalone/i2c/i2c.hpp
)

target_link_libraries(${NAME}
    PUBLIC
        emlib
        i2c
)

target_include_directories(${NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/Include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/standalone/i2c
)