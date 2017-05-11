include(common.cmake)
set(UNIT_TEST_EXECUTABLES "" CACHE INTERNAL "unit tests list" FORCE)

add_subdirectory(base)

add_subdirectory(properties)
add_subdirectory(mission)
add_subdirectory(drivers)
add_subdirectory(communication)
add_subdirectory(state)
add_subdirectory(others)

message(STATUS "Unit tests=${UNIT_TEST_EXECUTABLES}")

add_custom_target(unit_tests ALL DEPENDS ${UNIT_TEST_EXECUTABLES})

foreach(target ${UNIT_TEST_EXECUTABLES})
    list(APPEND UNIT_TEST_RUNS ${target}.run)
endforeach()

add_custom_target(unit_tests.run DEPENDS ${UNIT_TEST_RUNS})

add_custom_target(unit_tests.coverage
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
           DEPENDS unit_tests.run
        )
