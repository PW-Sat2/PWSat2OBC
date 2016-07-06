INCLUDE(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)

find_program(CC NAMES arm-none-eabi-gcc PATHS $ENV{ARM_TOOLCHAIN})
find_program(CXX NAMES arm-none-eabi-g++ PATHS $ENV{ARM_TOOLCHAIN})

find_program(CMAKE_AR NAMES arm-none-eabi-ar PATHS $ENV{ARM_TOOLCHAIN})
find_program(CMAKE_OBJCOPY NAMES arm-none-eabi-objcopy PATHS $ENV{ARM_TOOLCHAIN})
find_program(CMAKE_GCC_SIZE NAMES arm-none-eabi-size PATHS $ENV{ARM_TOOLCHAIN})
find_program(CMAKE_MAKE_PROGRAM NAMES make PATHS $ENV{ARM_TOOLCHAIN})

CMAKE_FORCE_C_COMPILER(${CC} GNU)
CMAKE_FORCE_CXX_COMPILER(${CXX} GNU)

find_program(QEMU NAMES qemu-system-gnuarmeclipse PATHS $ENV{ARM_TOOLCHAIN} $ENV{QEMU_PATH})
find_program(JLINK NAMES JLink JLinkExe PATHS $ENV{ARM_TOOLCHAIN} $ENV{JLINK_PATH})

FIND_PACKAGE(PythonInterp 2.7)
FIND_PACKAGE(Doxygen)

find_program(CLANG_FORMAT NAMES clang-format HINTS $ENV{CLANG_PATH})
