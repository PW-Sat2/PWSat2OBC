set(NAME version)

generate_version_file(${CMAKE_CURRENT_BINARY_DIR}/gen/version.h)

add_library(${NAME} INTERFACE)
message(STATUS DIR: "${CMAKE_CURRENT_BINARY_DIR}/gen") 
target_include_directories(${NAME} INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/gen")
