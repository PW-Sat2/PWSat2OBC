# Download and unpack rapidcheck at configure time
configure_file(CMakeLists.txt.in ${CMAKE_CURRENT_BINARY_DIR}/rapidcheck-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rapidcheck-download
    )

execute_process(COMMAND ${CMAKE_COMMAND} --build .  
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/rapidcheck-download 
    )

set(RC_ENABLE_GMOCK ON CACHE BOOL "" FORCE)
set(RC_ENABLE_GTEST ON CACHE BOOL "" FORCE)
add_compile_options(-fexceptions)
add_compile_options(-funwind-tables)
add_compile_options(-Wno-unused-parameter)
#add_compile_options(-frtti)

# Add rapidcheck directly to our build. This adds
# the following targets: rapidcheck rapidcheck_gtest rapidcheck_gmock
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/rapidcheck-src ${CMAKE_CURRENT_BINARY_DIR}/rapidcheck-build)
