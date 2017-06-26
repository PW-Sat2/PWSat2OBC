set(NAME gpio)

set(SOURCES
    gpio.cpp
    InterruptPinDriver.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} PUBLIC
	emlib
	base
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/${NAME})
