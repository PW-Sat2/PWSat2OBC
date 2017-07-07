cmake_minimum_required(VERSION 3.1)

include(toolchain.cmake)
include(utils/functions.cmake)

project(PWSat C CXX ASM)

option(ENABLE_LTO "Use link time optimization" OFF)

set(ENABLE_COVERAGE FALSE CACHE BOOL "Enable code coverage")

set(MEM_MANAGMENT_TYPE 1)

set(TARGET_MCU_PLATFORM "EngModel" CACHE STRING "Target mcu platform")
set(TARGET_PLD_PLATFORM "Mock" CACHE STRING "Target payload platform")
set(SEMIHOSTING false CACHE BOOL "Enable semihosting")
set(JLINK_SN "" CACHE STRING "J-Link serial number")

set(MCU_PLATFORM_PATH ${CMAKE_CURRENT_SOURCE_DIR}/platforms/mcu/${TARGET_MCU_PLATFORM})
set(PLD_PLATFORM_PATH ${CMAKE_CURRENT_SOURCE_DIR}/platforms/payload/${TARGET_PLD_PLATFORM})

set(OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/build/${TARGET_MCU_PLATFORM}/${TARGET_PLD_PLATFORM})
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

if(EXISTS ${MCU_PLATFORM_PATH})
else()
  message(FATAL_ERROR "Target mcu platform '${MCU_PLATFORM_PATH}' does not exist")
endif()

if(EXISTS ${PLD_PLATFORM_PATH})
else()
  message(FATAL_ERROR "Target payload platform '${PLD_PLATFORM_PATH}' does not exist")
endif()

include(${MCU_PLATFORM_PATH}/settings.cmake)
add_subdirectory(${MCU_PLATFORM_PATH})
add_subdirectory(${PLD_PLATFORM_PATH})

add_subdirectory(src)
add_subdirectory(libs)
add_subdirectory(doc)
add_subdirectory(unit_tests)
add_subdirectory(integration_tests)
add_subdirectory(utils)
add_subdirectory(test)
add_subdirectory(boot)
add_subdirectory(sample)
add_subdirectory(tools)
add_subdirectory(safe_mode)

message(STATUS "Using C compiler from ${CMAKE_C_COMPILER}")
message(STATUS "Target mcu platform ${TARGET_MCU_PLATFORM}")
message(STATUS "Target payload platform ${TARGET_PLD_PLATFORM}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Code coverage: ${ENABLE_COVERAGE}")
if(NOT ${JLINK_SN} STREQUAL "")
    message(STATUS "J-Link serial number: ${JLINK_SN}")
endif()

message(STATUS "Boot index: ${BOOT_INDEX}")
message(STATUS "Boot to upper: ${BOOT_USE_UPPER}")

