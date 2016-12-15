set(NAME emdrv)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Source SOURCES)

add_library(${NAME} STATIC ${SOURCES})
target_link_libraries(${NAME} emlib)

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)


