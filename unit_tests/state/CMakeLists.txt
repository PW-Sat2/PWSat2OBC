set(NAME unit_tests_state)

set(SOURCES
  state/PersistentStateTest.cpp
  state/PersistentTimeStateTest.cpp
  state/AntennaConfigTest.cpp
  state/TelemetryTest.cpp
  obc/ObcStateTest.cpp 
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME}
    obc_state
    unit_tests_base
    state
)


