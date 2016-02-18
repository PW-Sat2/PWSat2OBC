include(ExternalProject)

set(OPTIONS "${CMAKE_CXX_FLAGS} -DGTEST_HAS_POSIX_RE=0 -DGTEST_HAS_STREAM_REDIRECTION=0 -DGTEST_MOCK_FS=1")

ExternalProject_Add(gtest_source
  GIT_REPOSITORY https://github.com/google/googletest.git
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
  UPDATE_DISCONNECTED 1
  INSTALL_COMMAND ""
  PATCH_COMMAND
    git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/0001-Allow-mocking-file-system-getcmd-mkdir.patch
    && git apply ${CMAKE_CURRENT_SOURCE_DIR}/patches/0002-Fix-passing-CXX-flags.patch
  CMAKE_ARGS
    -DBUILD_GMOCK=OFF
    -DBUILD_GTEST=ON
    -DCMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/toolchain.cmake
    -DCMAKE_CXX_FLAGS=${OPTIONS}
    -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
    -DCMAKE_BUILD_TYPE:STRING=$<CONFIG>
  EXCLUDE_FROM_ALL 1
)

ExternalProject_Get_Property(gtest_source source_dir)
ExternalProject_Get_Property(gtest_source binary_dir)

function(addExternalLib name dependency source_location binary_location include_directory)
    add_library(${name} INTERFACE)
    add_dependencies(${name} ${dependency})
    target_link_libraries(${name} INTERFACE "${binary_location}/${CMAKE_FIND_LIBRARY_PREFIXES}${name}${CMAKE_STATIC_LIBRARY_SUFFIX}") 
    target_include_directories(${name} INTERFACE "${include_directory}")
endfunction(addExternalLib)

addExternalLib(gtest gtest_source "${source_dir}/googletest" "${binary_dir}/googletest" "${source_dir}/googletest/include")
