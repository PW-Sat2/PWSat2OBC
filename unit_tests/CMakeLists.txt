include(common.cmake)
set(UNIT_TEST_EXECUTABLES "" CACHE INTERNAL "unit tests list" FORCE)

add_subdirectory(base)
add_subdirectory(empty)
add_subdirectory(drivers)
add_subdirectory(communication)
add_subdirectory(mission)
add_subdirectory(state)

add_subdirectory(properties)

add_subdirectory(others)

message(STATUS "Unit tests=${UNIT_TEST_EXECUTABLES}")

add_custom_target(unit_tests ALL DEPENDS ${UNIT_TEST_EXECUTABLES})

foreach(target ${UNIT_TEST_EXECUTABLES})
    list(APPEND UNIT_TEST_RUNS ${target}.run)
endforeach()

add_custom_target(unit_tests.run ALL DEPENDS ${UNIT_TEST_RUNS})
