set(NAME program_flash)

set(SOURCES
    boot_table.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	bspFlash
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/${NAME})
