set(NAME exp_radfet)

set(SOURCES
    radfet.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	experiments
	time
	power
	payload
	exp_fs
	fs
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/experiment/radfet)
