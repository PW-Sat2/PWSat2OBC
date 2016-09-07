set(NAME integration_tests)

set(MOCK_COM "XXX-MOCK" CACHE STRING "Serial port for system bus mock")
set(PAYLOAD_COM "XXX-PAYLOAD" CACHE STRING "Serial port for payload bus mock")
set(OBC_COM "XXX-OBC" CACHE STRING "Serial port for OBC terminal")

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
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}" ipython -i pyconsole.py
    DEPENDS ${NAME}.deps
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.py.template ${CMAKE_CURRENT_BINARY_DIR}/config.py)
