set(NAME mission)

set(SOURCES    
    Include/mission/base.hpp
    Include/mission/logic.hpp
    Include/mission/main.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    time
    logger
    gsl
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)

target_format_sources(${NAME} "${SOURCES}")

add_subdirectory(obc)
add_subdirectory(adcs)
add_subdirectory(antenna)
add_subdirectory(sail)
add_subdirectory(time)
add_subdirectory(experiments)
add_subdirectory(beacon)
add_subdirectory(state)
add_subdirectory(telemetry)
