include(ExternalProject)

set(OPTIONS "${CMAKE_CXX_FLAGS} --exceptions -funwind-tables")

ExternalProject_Add(rapidcheck_source
  GIT_REPOSITORY https://github.com/Novakov/rapidcheck.git
  GIT_TAG newlib
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/rapidcheck
  UPDATE_DISCONNECTED 1
  INSTALL_COMMAND ""
  CMAKE_ARGS   
    -DCMAKE_TOOLCHAIN_FILE=${PROJECT_SOURCE_DIR}/toolchain.cmake
    -DCMAKE_CXX_FLAGS=${OPTIONS}
    -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
    -DCMAKE_BUILD_TYPE:STRING=$<CONFIG>
  LOG_CONFIGURE ON
  LOG_BUILD ON
  EXCLUDE_FROM_ALL 1
)

ExternalProject_Get_Property(rapidcheck_source source_dir)
ExternalProject_Get_Property(rapidcheck_source binary_dir)

function(addExternalLib name dependency source_location binary_location include_directory)
    add_library(${name} INTERFACE)
    add_dependencies(${name} ${dependency})
    target_link_libraries(${name} INTERFACE "${binary_location}/${CMAKE_FIND_LIBRARY_PREFIXES}${name}${CMAKE_STATIC_LIBRARY_SUFFIX}")
    target_include_directories(${name} INTERFACE "${include_directory}")
endfunction(addExternalLib)

addExternalLib(rapidcheck rapidcheck_source "${source_dir}" "${binary_dir}" "${source_dir}/include")

add_library(rapidcheck.gtest INTERFACE)
target_include_directories(rapidcheck.gtest INTERFACE "${source_dir}/extras/gtest/include")

add_library(rapidcheck.gmock INTERFACE)
target_include_directories(rapidcheck.gmock INTERFACE "${source_dir}/extras/gmock/include")