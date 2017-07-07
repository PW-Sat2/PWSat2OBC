set(USE_EXTERNAL_FLASH FALSE CACHE BOOL "Use external N25Q flash memory")
set(COMM_SECURITY_CODE "0xBBCCDDEE" CACHE STRING "32-bit COMM security code written in hex with leading 0x")

add_definitions(-DEFM32GG990F1024)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    add_definitions(-DENABLE_ASSERT -DDEBUG_EFM_USER)
endif()

if(${USE_EXTERNAL_FLASH})
    add_definitions(-DUSE_EXTERNAL_FLASH)
    message(STATUS "Flash: External (N25Q)")
else()
    message(STATUS "Flash: Internal (STK NAND)")
endif()

add_definitions(-DUSE_LEUART)
add_definitions(-DSLOWER_CLOCK)

set (ARCH cortex-m3)
set (CHIP EFM32GG)
set (DEVICE EFM32GG990F1024)
set (QEMU_MCU ${DEVICE}_BIG)
set (SAFE_MODE_LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/efm32gg.ld)

if(${ENABLE_COVERAGE})
    set (CCOVERAGE "-fprofile-arcs -ftest-coverage")
else()
    set (CCOVERAGE )
endif()

if(ENABLE_LTO)
    set(LTO_SWITCH "-flto")
endif()

set (CWARN "-Wall -Wstrict-prototypes -Wextra -Werror")
set (CXXWARN "-Wall -Wextra -Werror")
set (CTUNING "-ggdb -pedantic -fomit-frame-pointer -ffunction-sections -fdata-sections")
set (CMCU "-mtune=cortex-m3 -MMD -MP -mcpu=cortex-m3 -mthumb -march=armv7-m -mlittle-endian -mfix-cortex-m3-ldrd -mno-thumb-interwork")
set (CMAKE_C_FLAGS "-std=gnu11 ${CWARN} ${CTUNING} ${CMCU} ${CCOVERAGE}")
set (CMAKE_CXX_FLAGS "-std=gnu++1y -fno-exceptions ${CXXWARN} ${CTUNING} ${CMCU} ${CCOVERAGE}")
set (CMAKE_CXX_STANDARD 14)

set(DEBUG_COMP_OPTIONS "-DDEBUG -O0 -g")
set(RELEASE_COMP_OPTIONS "-DNDEBUG -O2 ${LTO_SWITCH}")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${LTO_SWITCH}")

set(CMAKE_C_FLAGS_DEBUG ${DEBUG_COMP_OPTIONS})
set(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_COMP_OPTIONS})
set(CMAKE_C_FLAGS_RELEASE ${RELEASE_COMP_OPTIONS})
set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_COMP_OPTIONS})

set(LOWER_LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/efm32gg.ld)
set(UPPER_LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/efm32gg.ld)

set(LOWER_ADDRESS 0x0)
set(UPPER_ADDRESS 0x0)