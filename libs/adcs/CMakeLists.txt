set(NAME adcs)

set(SOURCES      
    AdcsCoordinator.cpp   
    Include/adcs/adcs.hpp
    Include/adcs/AdcsCoordinator.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base
    logger
    time
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/adcs)

target_format_sources(${NAME} "${SOURCES}")

add_subdirectory(builtin)
add_subdirectory(experimental)
