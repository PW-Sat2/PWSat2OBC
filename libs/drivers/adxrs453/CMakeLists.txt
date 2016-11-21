set(NAME adxrs453)

set(SOURCES
   adxrs453.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} base platform logger emlib emdrv)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/adxrs453)

target_format_sources(${NAME} "${SOURCES}")
