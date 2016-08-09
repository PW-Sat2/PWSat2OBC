set(NAME emdrv)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Source SOURCES)

add_library(${NAME} STATIC ${SOURCES})
target_link_libraries(${NAME} emlib)

target_compile_definitions(${NAME} PRIVATE -D__START=main -D__STARTUP_CLEAR_BSS)

target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Include)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

