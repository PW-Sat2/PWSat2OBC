set(NAME state)

set(SOURCES
    Include/state/antenna/AntennaConfiguration.hpp
    Include/state/antenna/AntennaState.hpp
    Include/state/struct.h
    Include/state/PersistentState.hpp
    Include/state/fwd.hpp
    state.cpp
    PersistentState.cpp
    Antenna.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    time
    adcs
    gsl
    experiments
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/state)

target_format_sources(${NAME} "${SOURCES}")