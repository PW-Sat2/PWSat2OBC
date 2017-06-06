set(NAME telemetry)

set(SOURCES
    Include/telemetry/state.hpp
    SystemStartup.cpp
    ProgramState.cpp
    ErrorCounters.cpp
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
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_format_sources(${NAME} "${SOURCES}")

add_subdirectory(comm)
add_subdirectory(gyro)
add_subdirectory(fdir)
add_subdirectory(eps)
