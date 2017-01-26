set(NAME obc_adcs)

set(SOURCES
    ObcAdcs.cpp
    Include/adcs/ObcAdcs.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	builtin_detumbling
	adcs_experiment
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/adcs)
