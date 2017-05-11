set(NAME adcs_experimental)

set(SOURCES      
    DetumblingComputations.cpp
    ExperimentalDetumbling.cpp
    Include/adcs/DetumblingComputations.hpp
    Include/adcs/InterfaceTypes.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base    
    logger
    adcs
    eigen
    imtq
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/adcs)

target_format_sources(${NAME} "${SOURCES}")
