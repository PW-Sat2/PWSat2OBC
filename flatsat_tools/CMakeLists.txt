set(TOOLS_PATH ${OUTPUT_PATH}/tools)

set(SOURCES
    flash.py
    print_config.py
)

add_custom_target(flatsat_tools
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/flash.py ${TOOLS_PATH}/flash.py
)
