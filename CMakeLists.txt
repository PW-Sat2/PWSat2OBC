cmake_minimum_required(VERSION 3.1)

include(toolchain.cmake)
include(utils/functions.cmake)

project(PWSat C CXX ASM)

set(ENABLE_COVERAGE FALSE CACHE BOOL "Enable code coverage" )

set(MEM_MANAGMENT_TYPE 1)

set(TARGET_PLATFORM "DevBoard" CACHE STRING "Target platform")
set(SEMIHOSTING false CACHE BOOL "Enable semihosting")

set(PLATFORM_PATH ${CMAKE_CURRENT_SOURCE_DIR}/platforms/${TARGET_PLATFORM})

set(OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/build/${TARGET_PLATFORM})
set(REPORTS_PATH ${OUTPUT_PATH}/reports)

file(MAKE_DIRECTORY ${OUTPUT_PATH})

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH}/bin)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
else()
    set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "" FORCE)
endif()

if(EXISTS ${PLATFORM_PATH})
else()
  message(FATAL_ERROR "Target platform ${TARGET_PLATFORM} does not exist")
endif()

include(${PLATFORM_PATH}/settings.cmake)
add_subdirectory(${PLATFORM_PATH})


message(STATUS "Using C compiler from ${CMAKE_C_COMPILER}")
message(STATUS "Target platform ${TARGET_PLATFORM}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Code coverage: ${ENABLE_COVERAGE}")

add_subdirectory(src)
add_subdirectory(libs)
add_subdirectory(doc)
add_subdirectory(unit_tests)
add_subdirectory(integration_tests)
add_subdirectory(utils)
add_subdirectory(test)
