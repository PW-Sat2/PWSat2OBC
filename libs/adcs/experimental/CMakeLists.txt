set(NAME adcs_experimental)

set(SOURCES      
    AdcsExperimental.cpp
    Include/adcs/AdcsExperimental.hpp
    Include/adcs/InterfaceTypes.hpp
    Detumbling.cpp
    Include/adcs/Detumbling.hpp
    SunPointing.cpp
    Include/adcs/SunPointing.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base    
    logger
    adcs
    eigen
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/adcs)

target_format_sources(${NAME} "${SOURCES}")
