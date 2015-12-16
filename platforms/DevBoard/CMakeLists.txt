aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Source CMSIS_SOURCE)
list(APPEND CMSIS_SOURCE ${CMAKE_CURRENT_LIST_DIR}/Source/startup_efm32gg.S)

add_library(platform STATIC ${CMSIS_SOURCE})

target_compile_definitions(platform PRIVATE -D__START=main -D__STARTUP_CLEAR_BSS)

target_include_directories (platform PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Include)
