set(NAME builtin_detumbling)

set(SOURCES      
	BuiltinDetumbling.cpp
    Include/adcs/BuiltinDetumbling.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME}
    base    
    logger
    adcs
    imtq
    power
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/adcs)

target_format_sources(${NAME} "${SOURCES}")
