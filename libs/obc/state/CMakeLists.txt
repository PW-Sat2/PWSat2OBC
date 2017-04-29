set(NAME obc_state)

set(SOURCES
    Include/obc/IStorageAccess.hpp
    Include/obc/ObcState.hpp
    ObcState.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	fm25w
	state
	logger
	spi
	mission_state
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/obc)
