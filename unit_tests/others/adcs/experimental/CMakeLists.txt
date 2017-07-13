# pass some configuration to sources

set(DETUMBLIG_UT_DATA_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/data)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/adcsDetumblingUtConfig.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/adcsDetumblingUtConfig.h"
  )
  
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
