set(NAME bspLib)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src SOURCES)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	emlib
	bspFlash
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/bsp)

target_compile_definitions(${NAME} PUBLIC -DCubeCompV3)

set_target_properties(${NAME} PROPERTIES COMPILE_FLAGS "\
    -Wno-unused-parameter \
    -Wno-strict-prototypes \
    -Wno-unused-but-set-variable \
    -Wno-old-style-declaration \
")