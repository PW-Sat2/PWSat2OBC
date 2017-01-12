set(NAME obc_storage)

add_library(${NAME} INTERFACE)

target_link_libraries(${NAME} INTERFACE
    fs
    yaffs
    n25q
    storage
    obc_hardware
)

if(USE_EXTERNAL_FLASH)
    add_subdirectory(n25q_flash)
    target_link_libraries(${NAME} INTERFACE obc_storage_n25q)
else()
    add_subdirectory(stk_flash)
    target_link_libraries(${NAME} INTERFACE obc_storage_stk)
endif()

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
