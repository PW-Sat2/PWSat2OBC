set(TOOLS_PATH ${OUTPUT_PATH}/tools)

set(SOURCES
    flash.py
    print_config.py
    run_tests.py
)

add_custom_target(flatsat_tools)

foreach(F ${SOURCES})
    add_custom_command(TARGET flatsat_tools   
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${F} ${TOOLS_PATH}/${F}
    )
endforeach()