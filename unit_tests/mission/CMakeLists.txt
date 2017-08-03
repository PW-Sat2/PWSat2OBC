set(NAME unit_tests_mission)

set(SOURCES
  MissionPlan/MissionPlanTest.cpp
  MissionPlan/TimeTaskTest.cpp
  MissionPlan/MissionLoopTest.cpp
  MissionPlan/TelemetryTest.cpp
  MissionPlan/FileSystemTaskTest.cpp
  MissionPlan/antenna/DeployAntennasTest.cpp
  MissionPlan/antenna/AntennaMissionStateTest.cpp
  MissionPlan/antenna/StopAntennaDeploymentTest.cpp
  MissionPlan/beacon/BeaconUpdateTest.cpp
  MissionPlan/CompositeActionTest.cpp
  MissionPlan/adcs/AdcsPrimaryTaskTest.cpp
  MissionPlan/obc/ObcTest.cpp
  MissionPlan/sail/OpenSailTest.cpp
  MissionPlan/SendMessageTest.cpp
  Experiments/Fibo/FiboCalculatorTest.cpp
  Experiments/ExperimentTest.cpp
  Experiments/ADCS/DetumblingExperimentTest.cpp
  Experiments/LEOP/LEOPExperimentTest.cpp
  Experiments/fs/ExperimentFileTest.cpp
  Experiments/SunS/SunSExperimentTest.cpp  
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME} 
    mission
    mission_antenna
    mission_comm
    mission_sail
    mission_time
    mission_experiments
    mission_adcs
    mission_beacon
    mission_telemetry
    mission_fs
    exp_fibo
    exp_adcs
    exp_leop
    exp_fs
    exp_suns
    unit_tests_base
)

