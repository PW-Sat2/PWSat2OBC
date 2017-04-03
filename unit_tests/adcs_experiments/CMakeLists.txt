# pass some configuration to sources

message(WARNING "message to display")

set(DATA_FILE_PATH ${PROJECT_SOURCE_DIR}/unit_tests/adcs_experiments/data)

configure_file (
  "${PROJECT_SOURCE_DIR}/unit_tests/adcs_experiments/adcsDetumblingConfig.h.in"
  "${PROJECT_BINARY_DIR}/adcsDetumblingConfig.h"
  )
  
  include_directories("${PROJECT_BINARY_DIR}")