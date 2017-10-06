set(NAME integration_tests)

set(MOCK_COM "STM_MOCK" CACHE STRING "Serial port of STM mock")
set(OBC_COM "XXX-OBC" CACHE STRING "Serial port for OBC terminal")
set(GPIO_COM "XXX-GPIO" CACHE STRING "Serial port for GPIO")

set(BOOT_INDEX 5 CACHE NUMBER "Boot index used during integration tests")
set(BOOT_USE_UPPER TRUE CACHE BOOL "Boot directly to upper half of flash bypassing load from external memory")

set(TEST_RESULTS ${OUTPUT_PATH}/integration-tests.xml)

add_custom_target(${NAME}.deps
    COMMAND ${PYTHON_EXECUTABLE} -m pip -q install -r requirements.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(${NAME}
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}" ${PYTHON_EXECUTABLE} -m nose -x -s --nologcapture --rednose -v --with-xunit "--xunit-file=${TEST_RESULTS}"
    DEPENDS pwsat.flash ${NAME}.deps generate_telemetry.run generate_exp_data.run
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(pyconsole
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}" ${IPYTHON} --profile-dir=${CMAKE_CURRENT_SOURCE_DIR}/ipython_profile --gui=wx
    DEPENDS ${NAME}.deps
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

if(${BOOT_USE_UPPER})
    set(BOOT_USE_UPPER_PY "True")
else()
    set(BOOT_USE_UPPER_PY "False")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.py.template ${CMAKE_CURRENT_BINARY_DIR}/config.py)
