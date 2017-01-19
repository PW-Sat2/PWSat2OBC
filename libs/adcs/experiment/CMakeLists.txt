set(NAME adcs_experiment)

set(SOURCES      
    AdcsExperiment.cpp
    Include/adcs/AdcsExperiment.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base    
    logger
    adcs
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/adcs)

target_format_sources(${NAME} "${SOURCES}")