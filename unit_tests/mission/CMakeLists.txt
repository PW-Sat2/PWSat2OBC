set(NAME unit_tests_mission)

set(SOURCES
  MissionPlan/MissionPlanTest.cpp
  MissionPlan/TimeTaskTest.cpp
  MissionPlan/MissionLoopTest.cpp
  MissionPlan/TelemetryTest.cpp
  MissionPlan/FileSystemTaskTest.cpp
  MissionPlan/beacon/BeaconUpdateTest.cpp
  MissionPlan/CompositeActionTest.cpp
  MissionPlan/comm/CommWdogTest.cpp
  MissionPlan/SendMessageTest.cpp
  MissionPlan/PeriodicPowerCycleTest.cpp
  Experiments/ExperimentTest.cpp
  Experiments/fs/ExperimentFileTest.cpp
  Experiments/SunS/SunSExperimentTest.cpp 
  Experiments/EraseFlash/EraseFlashExperimentTest.cpp 
  Experiments/payload/PayloadExperimentTest.cpp
  Experiments/RadFET/RadFETExperimentTest.cpp 
  Experiments/CopyBootSlots/CopyBootSlotsExperimentTest.cpp
  BeaconSenderTest.cpp
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME} 
    mission
    mission_comm
    mission_time
    mission_experiments
    mission_beacon
    mission_telemetry
    mission_fs
    mission_power
    exp_fs
    exp_suns
    exp_flash
    exp_pld
    exp_radfet
    exp_program
    unit_tests_base
    payload
    beacon
)

