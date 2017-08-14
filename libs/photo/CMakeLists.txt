set(NAME photo)

set(SOURCES
    photo_service.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	power
	fs
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/${NAME})
