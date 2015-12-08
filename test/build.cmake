set(NAME ut)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} UT_SOURCES)

add_executable(${NAME} ${UT_SOURCES})

target_link_libraries(${NAME} ${GTEST_LIB})
target_include_directories(${NAME} PRIVATE ${GTEST_INCLUDE})

foreach(lib platform)
  target_link_libraries(${NAME} ${lib})
  add_dependencies(${NAME} ${lib})
endforeach(lib)

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
