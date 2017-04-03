# pass some configuration to sources

set(DETUMBLIG_UT_DATA_FILE_PATH ${PROJECT_SOURCE_DIR}/unit_tests/adcs_experiments/data)

configure_file (
  "${PROJECT_SOURCE_DIR}/unit_tests/adcs_experiments/adcsDetumblingUtConfig.h.in"
  "${PROJECT_BINARY_DIR}/unit_tests/adcsDetumblingUtConfig.h"
  )
  
  include_directories("${PROJECT_BINARY_DIR}")