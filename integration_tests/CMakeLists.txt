set(NAME integration_tests)

set(MOCK_COM "XXX-MOCK" CACHE STRING "Serial port for mock" )
set(OBC_COM "XXX-OBC" CACHE STRING "Serial port for OBC terminal")

set(TEST_RESULTS ${OUTPUT_PATH}/integration-tests.xml)

add_custom_target(${NAME}.deps
    COMMAND ${PYTHON_EXECUTABLE} -m pip install -r requirements.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(${NAME}
    COMMAND ${CMAKE_COMMAND} -E env "MOCK_COM=${MOCK_COM}" "OBC_COM=${OBC_COM}" ${PYTHON_EXECUTABLE} -m nose -v --with-xunit "--xunit-file=${TEST_RESULTS}"
    DEPENDS pwsat.flash ${NAME}.deps
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(pyconsole
    COMMAND ${CMAKE_COMMAND} -E env "MOCK_COM=${MOCK_COM}" "OBC_COM=${OBC_COM}" ipython -i pyconsole.py
    DEPENDS ${NAME}.deps
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
