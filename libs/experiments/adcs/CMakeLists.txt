set(NAME exp_adcs)

set(SOURCES
    detumbling.cpp
    Include/experiment/adcs/adcs.hpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	adcs
	experiments
	time
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/experiment/adcs)
