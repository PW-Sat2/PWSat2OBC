set(NAME unit_tests_mission)

set(SOURCES
  MissionPlan/MissionPlanTest.cpp
  MissionPlan/SailSystemStateTest.cpp
  MissionPlan/TimeTaskTest.cpp
  MissionPlan/MissionLoopTest.cpp
  MissionPlan/antenna/DeployAntennasTest.cpp
  MissionPlan/antenna/AntennaMissionStateTest.cpp
  MissionPlan/beacon/BeaconUpdateTest.cpp
  MissionPlan/CompositeActionTest.cpp
  MissionPlan/adcs/AdcsPrimaryTaskTest.cpp
  MissionPlan/obc/ObcTest.cpp    
  Experiments/Fibo/FiboCalculatorTest.cpp
  Experiments/ExperimentTest.cpp
  Experiments/ADCS/DetumblingExperimentTest.cpp
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME} 
    mission
    mission_antenna
    mission_sail
    mission_time
    mission_experiments
    mission_adcs
    mission_beacon   
    exp_fibo
    exp_adcs
    unit_tests_base
)
