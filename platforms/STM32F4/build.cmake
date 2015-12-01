set (ARCH cortex-m3)
set (CHIP STM32F4)

set (CSTANDARD "-std=gnu99 --specs=nosys.specs")
set (CWARN "-Wall -Wstrict-prototypes -Wextra")
set (CTUNING "-pedantic -O0 -fPIC -g")
set (CMCU "-mtune=cortex-m3 -MMD -MP -mcpu=cortex-m3 -mthumb -march=armv7-m -mlittle-endian -mfix-cortex-m3-ldrd")
set (CDEFS "-DDEBUG -DUSE_STDPERIPH_DRIVER")
set (CMAKE_C_FLAGS "${CSTANDARD} ${CWARN} ${CTUNING} ${CMCU} ${CDEFS}")
set (CMAKE_CXX_FLAGS "${CSTANDARD} ${CMCU}")

set(LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/Source/STM32F407xG.ld)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Source CMSIS_SOURCE)
list(APPEND CMSIS_SOURCE ${CMAKE_CURRENT_LIST_DIR}/Source/startup_stm32f4xx.s)

include_directories (${CMAKE_CURRENT_LIST_DIR}/Include)

add_library(platform STATIC ${CMSIS_SOURCE})
