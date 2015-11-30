cmake_minimum_required (VERSION 2.6)
project (demo-cortex-m3)

# The version number.
set (PWSat_VERSION_MAJOR 1)
set (PWSat_VERSION_MINOR 0)

set (ARCH cortex-m3)
set (CHIP EFM32GG)
set (OUTPUT_NAME pwsat2)

message (STATUS "CMAKE_C_FLAGS:${CMAKE_C_FLAGS}")
if (CMAKE_CROSSCOMPILING)
    set (CSTANDARD "-std=gnu99 --specs=nosys.specs")
    set (CWARN "-Wall -Wstrict-prototypes -Wextra")
    set (CTUNING "-pedantic -O0 -fPIC -g")
    set (CMCU "-mtune=cortex-m3 -MMD -MP -mcpu=cortex-m3 -mthumb -march=armv7-m -mlittle-endian -mfix-cortex-m3-ldrd")
    set (CDEFS "-DEFM32GG280F1024 -DDEBUG")
    set (CMAKE_C_FLAGS "${CSTANDARD} ${CWARN} ${CTUNING} ${CMCU} ${CDEFS}")
    set (CMAKE_CXX_FLAGS "${CSTANDARD} ${CMCU}")
endif (CMAKE_CROSSCOMPILING)

# Memory managment
set (MEM_MANAGMENT_TYPE 4)

# Platform
set (PLATFORM_SOURCE_DIR ${ROOT_DIR}/Platforms/Platform-${CHIP})
set (PLATFORM_BINARY_DIR ${CMAKE_BINARY_DIR}/.Platforms/Platform-${CHIP})

# FreeRTOS
set (FREERTOS_SOURCE_DIR ${ROOT_DIR}/FreeRTOS)
set (FREERTOS_BINARY_DIR ${CMAKE_BINARY_DIR}/.FreeRTOS)

# Applications
# Common
set (APP_COMMON_SOURCE_DIR ${ROOT_DIR}/Applications/Common)
set (APP_COMMON_BINARY_DIR ${CMAKE_BINARY_DIR}/.App/Common)
# Demo
set (APP_SOURCE_DIR ${ROOT_DIR}/Applications/App/PWSat2)
set (APP_BINARY_DIR ${CMAKE_BINARY_DIR}/.App/App/PWSat2)

# Define directory for headers
set (INCLUDE_DIR Include)
set (SOURCE_DIR Source)
set (DEST_INCLUDE_DIR ${CMAKE_BINARY_DIR}/${INCLUDE_DIR})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Create directories
file(MAKE_DIRECTORY ${PLATFORM_BINARY_DIR})
file(MAKE_DIRECTORY ${FREERTOS_BINARY_DIR})
file(MAKE_DIRECTORY ${APP_COMMON_BINARY_DIR})
file(MAKE_DIRECTORY ${APP_BINARY_DIR})
file(MAKE_DIRECTORY ${DEST_INCLUDE_DIR})


message (STATUS "FREERTOS_BINARY_DIR:" ${FREERTOS_BINARY_DIR})
message (STATUS "FREERTOS_SOURCE_DIR:" ${FREERTOS_SOURCE_DIR})

if (WIN32)
    message (STATUS "Operating system: Windows.")
elseif (UNIX)
    message (STATUS "Operating system: Unix.")
else ()
    message (STATUS "Unrecognized system.")
endif()

add_subdirectory(${PLATFORM_SOURCE_DIR} ${PLATFORM_BINARY_DIR})
add_subdirectory(${FREERTOS_SOURCE_DIR} ${FREERTOS_BINARY_DIR})
add_subdirectory(${APP_COMMON_SOURCE_DIR} ${APP_COMMON_BINARY_DIR})
add_subdirectory(${APP_SOURCE_DIR} ${APP_BINARY_DIR})

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OUTPUT_NAME})
set (HEX_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${OUTPUT_NAME}.hex)

add_custom_command(OUTPUT ${HEX_OBJ}
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXEC_OBJ} ${HEX_OBJ}
    DEPENDS ${OUTPUT_NAME})

add_custom_target (${OUTPUT_NAME}_hex ALL DEPENDS ${HEX_OBJ})
