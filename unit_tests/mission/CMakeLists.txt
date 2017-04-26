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

add_executable(${NAME} ${SOURCES})

if(${ENABLE_COVERAGE})
  set(LD_COVERAGE "-fprofile-arcs")
  target_link_libraries(${NAME} gcov)
  target_compile_definitions(${NAME} PRIVATE ENABLE_COVERAGE=1)
else()
  set(LD_COVERAGE )    
endif(${ENABLE_COVERAGE})

set_target_properties(${NAME} PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_LIST_DIR}/../base/linker.ld -u _printf_float -specs=rdimon.specs ${LD_COVERAGE}")

target_memory_report(${NAME})

target_asm_listing(${NAME})

target_compile_options(${NAME} PRIVATE "-fexceptions")

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

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})

add_custom_target(${NAME}.run
  COMMAND ${QEMU} -board generic -mcu ${QEMU_MCU} -nographic -monitor null -image ${EXEC_OBJ} -semihosting-config "arg=tests,arg=--gtest_output=xml:${OUTPUT_PATH}/unit-tests.xml"

  DEPENDS ${NAME}
)

if(${ENABLE_COVERAGE})
    add_custom_target(${NAME}.coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${REPORTS_PATH}/coverage
        COMMAND ${PYTHON_EXECUTABLE}
            -c "import gcovr.driver; gcovr.driver.main_()"
            "--gcov-executable=${CMAKE_GCOV}"
            "--root=${CMAKE_SOURCE_DIR}"
            "--object-directory=${CMAKE_BINARY_DIR}"
            "--exclude=unit_tests.*"
            "--exclude=libs.external.*"
            --html
            --html-details
            --print-summary
            --sort-percentage
            "--output=${REPORTS_PATH}/coverage/index.html"
       DEPENDS ${NAME}.run
    )
endif()

target_eclipse_debug_configs(${NAME} QEmu)

