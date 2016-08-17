add_definitions(-DEFM32GG990F1024 -DDEBUG_EFM_USER)

set (ARCH cortex-m3)
set (CHIP EFM32GG)
set (DEVICE EFM32GG990F1024)
set (QEMU_MCU ${DEVICE}_BIG)

set(CSPECS "-specs=nano.specs")

if(SEMIHOSTING)
  set (CSPECS "${CSPECS}")
  add_definitions(-DSEMIHOSTING=1)
else()
  set (CSPECS "${CSPECS} -specs=nosys.specs")
  add_definitions(-DSEMIHOSTING=0)
endif()

if(${ENABLE_COVERAGE})
    set (CCOVERAGE "-fprofile-arcs -ftest-coverage")
else()
    set (CCOVERAGE )
endif()

set (CWARN "-Wall -Wstrict-prototypes -Wextra")
set (CXXWARN "-Wall -Wextra")
set (CTUNING "-ggdb -pedantic -fomit-frame-pointer -ffunction-sections -fdata-sections")
set (CMCU "-mtune=cortex-m3 -MMD -MP -mcpu=cortex-m3 -mthumb -march=armv7-m -mlittle-endian -mfix-cortex-m3-ldrd -mno-thumb-interwork")
set (CMAKE_C_FLAGS "-std=gnu11 ${CSPECS} ${CWARN} ${CTUNING} ${CMCU} ${CCOVERAGE}")
set (CMAKE_CXX_FLAGS "-std=gnu++1y ${CSPECS} ${CXXWARN} ${CTUNING} ${CMCU} ${CCOVERAGE}")

set(DEBUG_COMP_OPTIONS "-DDEBUG -O0 -g")
set(RELEASE_COMP_OPTIONS "-DNDEBUG -O3")

set(CMAKE_C_FLAGS_DEBUG ${DEBUG_COMP_OPTIONS})
set(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_COMP_OPTIONS})
set(CMAKE_C_FLAGS_RELEASE ${RELEASE_COMP_OPTIONS})
set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_COMP_OPTIONS})

set(PLATFORM_LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/efm32gg.ld)
