add_custom_target(utils.deps
    COMMAND ${PYTHON_EXECUTABLE} -m pip -q install -r requirements.txt
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)