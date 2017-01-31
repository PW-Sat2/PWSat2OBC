include(ExternalProject)

#clone EIGEN library form git mirror
ExternalProject_Add(eigen_src
  GIT_REPOSITORY https://github.com/RLovelett/eigen.git
  GIT_TAG 3.3.2
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/eigen
  UPDATE_DISCONNECTED 1
  CONFIGURE_COMMAND ""
  BUILD_COMMAND  ""
  INSTALL_COMMAND ""
  EXCLUDE_FROM_ALL 1
)

ExternalProject_Get_Property(eigen_src source_dir)

function(addExternalLib name dependency include_directory)
  add_library(${name} INTERFACE)
  add_dependencies(${name} ${dependency})
  target_include_directories(${name} INTERFACE "${include_directory}")
endfunction(addExternalLib)

addExternalLib(eigen eigen_src "${source_dir}")

#block EIGEN using dynamic allocation of memory
target_compile_definitions(eigen INTERFACE EIGEN_NO_MALLOC)

