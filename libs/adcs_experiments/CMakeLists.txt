set(NAME adcs_experiments)

set(SOURCES      
    detumbling.cpp
    Include/adcs_experiments/detumbling.hpp
    Include/adcs_experiments/interface_types.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    eigen 
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/adcs_experiments)

target_format_sources(${NAME} "${SOURCES}")

