set(NAME state)

set(SOURCES
    Include/state/fwd.hpp
    state.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    time
    gsl
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/state)

target_format_sources(${NAME} "${SOURCES}")