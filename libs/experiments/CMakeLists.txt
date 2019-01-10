set(NAME experiments)

set(SOURCES
    experiments.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
    base
    gsl
    logger
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/${NAME})

add_subdirectory(fs)
add_subdirectory(suns)
add_subdirectory(flash)
add_subdirectory(payload)
add_subdirectory(radfet)
add_subdirectory(program)
