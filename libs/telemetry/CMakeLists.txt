set(NAME telemetry)

set(SOURCES
    Include/telemetry/state.hpp
    SystemStartup.cpp
    ErrorCounters.cpp
    ExperimentTelemetry.cpp
    state.cpp
    TimeTelemetry.cpp
    ImtqTelemetry.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    logger
    mission
    state
    comm
    gyro
    eps
    error_counter
    experiments
    fs
    antenna
    imtq
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_format_sources(${NAME} "${SOURCES}")

add_subdirectory(comm)
add_subdirectory(gyro)
add_subdirectory(fdir)
add_subdirectory(eps)
add_subdirectory(experiments)
add_subdirectory(serialization)
add_subdirectory(temp)
add_subdirectory(ant)
add_subdirectory(gpio)
add_subdirectory(fs)
add_subdirectory(time)
add_subdirectory(program)
add_subdirectory(flash_scrubbing)
add_subdirectory(ram_scrubbing)
