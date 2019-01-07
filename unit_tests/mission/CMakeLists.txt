set(NAME unit_tests_mission)

set(SOURCES
  MissionPlan/MissionPlanTest.cpp
  MissionPlan/TimeTaskTest.cpp
  MissionPlan/MissionLoopTest.cpp
  MissionPlan/TelemetryTest.cpp
  MissionPlan/FileSystemTaskTest.cpp
  MissionPlan/antenna/DeployAntennaTest.cpp
  MissionPlan/beacon/BeaconUpdateTest.cpp
  MissionPlan/CompositeActionTest.cpp
  MissionPlan/adcs/AdcsPrimaryTaskTest.cpp
  MissionPlan/obc/ObcTest.cpp
  MissionPlan/sads/DeploySolarArrayTest.cpp
  MissionPlan/comm/CommWdogTest.cpp
  MissionPlan/SendMessageTest.cpp
  MissionPlan/PeriodicPowerCycleTest.cpp
  Experiments/Fibo/FiboCalculatorTest.cpp
  Experiments/ExperimentTest.cpp
  Experiments/ADCS/DetumblingExperimentTest.cpp
  Experiments/LEOP/LEOPExperimentTest.cpp
  Experiments/fs/ExperimentFileTest.cpp
  Experiments/SunS/SunSExperimentTest.cpp 
  Experiments/EraseFlash/EraseFlashExperimentTest.cpp 
  Experiments/payload/PayloadExperimentTest.cpp
  Experiments/RadFET/RadFETExperimentTest.cpp 
  Experiments/Camera/CameraExperimentTest.cpp
  Experiments/CopyBootSlots/CopyBootSlotsExperimentTest.cpp
  BeaconSenderTest.cpp
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME} 
    mission
    mission_antenna
    mission_comm
    mission_sads
    mission_time
    mission_experiments
    mission_adcs
    mission_beacon
    mission_telemetry
    mission_fs
    mission_power
    exp_fibo
    exp_adcs
    exp_leop
    exp_fs
    exp_suns
    exp_flash
    exp_pld
    exp_radfet
    exp_camera
    exp_program
    unit_tests_base
    payload
    beacon
)

