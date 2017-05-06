set(NAME unit_tests_properties)

set(SOURCES
  EPS/EPSDriverTest.cpp  
  imtq/imtqTest.cpp
  MissionPlan/MissionTestHelpers.cpp
  MissionPlan/SailSystemStateTest.cpp
  gyro/gyroTest.cpp
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME}    
    eps
    fm25w
    imtq
    mission
    mission_sail
    rapidcheck
    rapidcheck_gtest
    rapidcheck_gmock
    unit_tests_base
    gyro
)
