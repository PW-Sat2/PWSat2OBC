set(NAME integration_tests)

set(MOCK_COM "XXX-MOCK" CACHE STRING "Serial port for mock" )
set(OBC_COM "XXX-OBC" CACHE STRING "Serial port for OBC terminal")

add_custom_target(${NAME} 
    COMMAND ${CMAKE_COMMAND} -E env "MOCK_COM=${MOCK_COM}" "OBC_COM=${OBC_COM}" ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/sail.py"
    DEPENDS pwsat.flash
)