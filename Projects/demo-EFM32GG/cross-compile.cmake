set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_SYSTEM_PROCESSOR arm)

include (CMakeForceCompiler)

#set(CMAKE_SYSROOT /home/devel/rasp-pi-rootfis)
#set(CMAKE_STAGING_PREFIX /home/devel/stage)

# CMake by default test chosen compiler.
# We configure compiler so there no need to run this test.

set(CMAKE_C_COMPILER_FORCED 1)
set(CMAKE_CXX_COMPILER_FORCED 1)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
#set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
#set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)

set(BUILD_SHARED_LIBS OFF)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXE_LINKER_FLAGS_RELEASE -static)

# FreeRTOS recommend to compile only with gnu99.
# -spec is required for arm-none-eabi-gcc compiler

#--------------------------------------------------------------------
#| ARM Core | Command Line Options                       | multilib |
#|----------|--------------------------------------------|----------|
#|Cortex-R4 | [-mthumb] -march=armv7-r                   | armv7-ar |
#|Cortex-R5 |                                            | /thumb   |
#|Cortex-R7 |                                            |          |
#|(No FP)   |                                            |          |
#|----------|--------------------------------------------|----------|
#|Cortex-R4 | [-mthumb] -march=armv7-r -mfloat-abi=softfp| armv7-ar |
#|Cortex-R5 | -mfpu=vfpv3-d16                            | /thumb   |
#|Cortex-R7 |                                            | /softfp  |
#|(Soft FP) |                                            |          |
#|----------|--------------------------------------------|----------|
#|Cortex-R4 | [-mthumb] -march=armv7-r -mfloat-abi=hard  | armv7-ar |
#|Cortex-R5 | -mfpu=vfpv3-d16                            | /thumb   |
#|Cortex-R7 |                                            | /fpu     |
#|(Hard FP) |                                            |          |
#--------------------------------------------------------------------
#

#set (CMAKE_C_FLAGS "-mcpu=cortex-r4 --std=gnu99 --specs=nosys.specs -mthumb -march=armv7-r -mtune=cortex-r4 -mfloat-abi=softfp -mfpu=vfpv3-d16 -fPIC -g -O1 -c")
set (CMAKE_C_FLAGS "-mcpu=cortex-r4 --std=gnu99 --specs=nosys.specs -mthumb -march=armv7-r -mtune=cortex-r4 -mlittle-endian -fno-exceptions -fno-builtin -mfloat-abi=softfp -mfpu=vfpv3-d16 -fPIC -g -O1 -c")

#set (CMAKE_CXX_FLAGS "-mcpu=cortex-r4 --std=gnu99 -mthumb -march=armv7-r -mtune=cortex-r4 -mfloat-abi=softfp -mfpu=vfpv3-d16 -fPIC")
set (CMAKE_CXX_FLAGS "-mcpu=cortex-r4 --std=gnu99 --specs=nosys.specs -mthumb -march=armv7-r -mtune=cortex-r4 -mlittle-endian -fno-exceptions -fno-builtin -mfloat-abi=softfp -mfpu=vfpv3-d16 -fPIC")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
