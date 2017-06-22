set(NAME emlib)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Source SOURCES)
list(APPEND SOURCES ${CMAKE_CURRENT_LIST_DIR}/Source/startup_efm32gg.S)

add_library(${NAME} STATIC ${SOURCES})

target_compile_definitions(${NAME} PRIVATE -D__START=main -D__STARTUP_CLEAR_BSS -DEM_MSC_RUN_FROM_FLASH)

target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Include)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

