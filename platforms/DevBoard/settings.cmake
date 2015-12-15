add_definitions(-DEFM32GG990F1024)
add_definitions(-D__START=main)

set (ARCH cortex-m3)
set (CHIP EFM32GG)
set (QEMU_MCU EFM32GG990F1024)

set (CSTANDARD "-specs=rdimon.specs -specs=nano.specs -lc -lnosys -lrdimon")
set (CWARN "-Wall -Wstrict-prototypes -Wextra")
set (CTUNING "-pedantic -O0 -fPIC -g")
set (CMCU "-mtune=cortex-m3 -MMD -MP -mcpu=cortex-m3 -mthumb -march=armv7-m -mlittle-endian -mfix-cortex-m3-ldrd")
set (CDEFS "-DEFM32GG280F1024 -DDEBUG")
set (CMAKE_C_FLAGS "-std=gnu99 ${CSTANDARD} ${CWARN} ${CTUNING} ${CMCU} ${CDEFS}")
set (CMAKE_CXX_FLAGS "${CSTANDARD} ${CMCU}")

set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/Source/efm32gg.ld)
