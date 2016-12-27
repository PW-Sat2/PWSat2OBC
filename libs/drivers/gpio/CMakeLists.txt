set(NAME gpio)

set(SOURCES

)

add_library(${NAME} INTERFACE ${SOURCES})

target_link_libraries(${NAME} INTERFACE
	emlib
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
#target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/${NAME})
