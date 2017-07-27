set(NAME unit_tests_state)

set(SOURCES
  state/PersistentStateTest.cpp
  state/PersistentTimeStateTest.cpp
  state/AntennaConfigTest.cpp
  state/TelemetryTest.cpp
  state/BootStateTest.cpp
  state/SailStateTest.cpp
  state/MessageStateTest.cpp
  obc/ObcStateTest.cpp 
  telemetry/CommTelemetryAcquisitionTest.cpp
  telemetry/SystemStartupTest.cpp
  telemetry/GyroTelemetryAcquisitionTest.cpp
  telemetry/ErrorCountingTelemetryTest.cpp
  telemetry/EpsTelemetryAcquisitionTest.cpp
  telemetry/ExperimentTelemetryTest.cpp
  telemetry/McuTemperatureTest.cpp
  telemetry/AntennaTelemetryAcquisitionTest.cpp
  telemetry/GpioStateTest.cpp
  telemetry/FileSystemTelemetryTest.cpp
  telemetry/FileSystemTelemetryAcquisitionTest.cpp
  telemetry/TimeTelemetryTest.cpp
  telemetry/TimeTelemetryAcquisitionTest.cpp
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME}
    obc_state
    unit_tests_base
    state
    telemetry
    telemetry_comm
    telemetry_gyro
    telemetry_eps
    telemetry_ant
    telemetry_fs
    telemetry_time
)


