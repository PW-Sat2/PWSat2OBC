include(ExternalProject)

ExternalProject_Add(gsl_source
  GIT_REPOSITORY https://github.com/Novakov/GSL.git
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gsl
  GIT_TAG a3e5a56c65168d9a91614986da2ca6fffa6805b9
  UPDATE_DISCONNECTED 1
  CONFIGURE_COMMAND ""
  INSTALL_COMMAND ""
  BUILD_COMMAND ""
  LOG_CONFIGURE ON
  EXCLUDE_FROM_ALL 1
)

ExternalProject_Get_Property(gsl_source source_dir)

function(addExternalLib name dependency include_directory)
    add_library(${name} INTERFACE)
    add_dependencies(${name} ${dependency})
    target_include_directories(${name} INTERFACE "${include_directory}")
endfunction(addExternalLib)

addExternalLib(gsl gsl_source "${source_dir}")
