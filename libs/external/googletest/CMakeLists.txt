# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in ${CMAKE_CURRENT_BINARY_DIR}/googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM} -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download
    )

execute_process(COMMAND ${CMAKE_COMMAND} --build .  
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download 
    )
 
# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_definitions(-DGTEST_HAS_POSIX_RE=0)
add_definitions(-DGTEST_HAS_STREAM_REDIRECTION=0)
add_definitions(-DGTEST_MOCK_FS=1)

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src ${CMAKE_CURRENT_BINARY_DIR}/googletest-build)
 
# The gtest/gmock targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  include_directories("${gtest_SOURCE_DIR}/include" 
                      "${gmock_SOURCE_DIR}/include")
endif()