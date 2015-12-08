INCLUDE(CMakeForceCompiler)

set(CMAKE_SYSTEM_NAME Generic)

set(TOOLCHAIN_ROOT "" CACHE STRING "Toolchain root")

if(WIN32)
  set(TOOL_EXT .exe)
else()
  set(TOOL_EXT )
endif()

CMAKE_FORCE_C_COMPILER(${TOOLCHAIN_ROOT}arm-none-eabi-gcc${TOOL_EXT} GNU)
CMAKE_FORCE_CXX_COMPILER(${TOOLCHAIN_ROOT}arm-none-eabi-g++${TOOL_EXT} GNU)

set(CMAKE_AR ${TOOLCHAIN_ROOT}arm-none-eabi-ar${TOOL_EXT} CACHE FILEPATH "Archiver")
set(CMAKE_OBJCOPY ${TOOLCHAIN_ROOT}arm-none-eabi-objcopy${TOOL_EXT} CACHE FILEPATH "Objcopy")
set(CMAKE_MAKE_PROGRAM ${TOOLCHAIN_ROOT}make${TOOL_EXT})
