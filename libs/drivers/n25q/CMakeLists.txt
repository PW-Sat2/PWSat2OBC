set(NAME n25q)

set(SOURCES
    n25q.cpp
    redundant_n25q.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	gsl
	spi
	yaffs
	logger
	fs
	assert
	error_counter
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/${NAME})
