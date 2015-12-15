include(ExternalProject)

set(OPTIONS "${CMAKE_CXX_FLAGS} -DGTEST_HAS_POSIX_RE=0 -DGTEST_HAS_STREAM_REDIRECTION=0 -DGTEST_MOCK_FS=1")

ExternalProject_Add(googletest
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
)

ExternalProject_Get_Property(googletest SOURCE_DIR BINARY_DIR)

# TODO: I would be nice to have target 'gtest' that will automatically add static library
# and include directory but... For some reason cmake add INTERFACE_INCLUDE_DIRECTORIES
# as system include directories which does not compile (template with C linkeage error)

# add_library(gtest STATIC IMPORTED GLOBAL)
# set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${BINARY_DIR}/googletest/libgtest.a)
# #add_dependencies(gtest googletest)
# #target_link_libraries(gtest INTERFACE ${BASE}/libgtest.a)
#
# set_property(TARGET gtest
#   PROPERTY
#     INTERFACE_INCLUDE_DIRECTORIES
#       ${SOURCE_DIR}/googletest/include
# )
#
# set_property(TARGET gtest
#   PROPERTY
#     IMPORTED_LINK_INTERFACE_LIBRARIES
#       ${BINARY_DIR}/googletest/libgtest.a
# )

set(GTEST_INCLUDE ${SOURCE_DIR}/googletest/include PARENT_SCOPE)
set(GTEST_LIB ${BINARY_DIR}/googletest/libgtest.a PARENT_SCOPE)
