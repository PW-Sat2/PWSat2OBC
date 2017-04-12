set(NAME mission_experiments)

set(SOURCES
    experiments.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    gsl
    mission
    state
    experiments
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)
