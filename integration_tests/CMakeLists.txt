set(NAME integration_tests)

set(MOCK_COM "STM_MOCK" CACHE STRING "Serial port of STM mock")
set(OBC_COM "XXX-OBC" CACHE STRING "Serial port for OBC terminal")
set(GPIO_COM "XXX-GPIO" CACHE STRING "Serial port for GPIO")

set(TEST_RESULTS ${OUTPUT_PATH}/integration-tests.xml)

add_custom_target(${NAME}.deps
    COMMAND ${PYTHON_EXECUTABLE} -m pip -q install -r requirements.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(${NAME}
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}" ${PYTHON_EXECUTABLE} -m nose -x -s --nologcapture --rednose -v --with-xunit "--xunit-file=${TEST_RESULTS}"
    DEPENDS pwsat.flash ${NAME}.deps
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(pyconsole
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}" ${IPYTHON} --profile-dir=${CMAKE_CURRENT_SOURCE_DIR}/ipython_profile
    DEPENDS ${NAME}.deps
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.py.template ${CMAKE_CURRENT_BINARY_DIR}/config.py)
