# pass some configuration to sources

set(ADCS_UT_DATA_FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/data)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/adcsUtConfig.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/adcsUtConfig.h"
  )
  
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
