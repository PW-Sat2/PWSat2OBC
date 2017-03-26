set(NAME obc_state)

set(SOURCES
    Include/obc/fwd.hpp
    Include/obc/IStorageAccess.hpp
    Include/obc/PersistentStorageAccess.hpp
    ObcState.cpp
    PersistentStorageAccess.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	fm25w
	state
	logger
	spi
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/obc)
