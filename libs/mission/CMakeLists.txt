set(NAME mission)

set(SOURCES    
    Include/mission/base.hpp
)

add_library(${NAME} INTERFACE)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
#target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mission)

target_format_sources(${NAME} "${SOURCES}")

add_subdirectory(antenna)
add_subdirectory(sail)
add_subdirectory(time)
