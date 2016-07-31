set(NAME time)

set(SOURCES
    time.c
    timer.c
    include/time/TimePoint.h
    include/time/timer.h
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} base fs logger)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/time)
target_format_sources(${NAME} "${SOURCES}")
