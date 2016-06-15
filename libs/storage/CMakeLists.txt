set(NAME storage)

set(SOURCES
    storage.c
    nand.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} logger platform emlib base) 

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/storage) 