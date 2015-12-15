cmake_minimum_required(VERSION 3.1)

include(./toolchain.cmake)

project(PWSat C CXX ASM)

set(MEM_MANAGMENT_TYPE 4)


set(TARGET_PLATFORM "DevBoard" CACHE STRING "Target platform")

set(PLATFORM_PATH ${CMAKE_CURRENT_SOURCE_DIR}/platforms/${TARGET_PLATFORM})

set(OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/build/${TARGET_PLATFORM})

file(MAKE_DIRECTORY ${OUTPUT_PATH})

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_PATH}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_PATH}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_PATH}/bin)

if(EXISTS ${PLATFORM_PATH})
else()
  message(FATAL_ERROR "Target platform ${TARGET_PLATFORM} does not exist")
endif()

include(${PLATFORM_PATH}/build.cmake)

set(CMAKE_EXE_LINKER_FLAGS "-T ${LINKER_SCRIPT} ${CMAKE_EXE_LINKER_FLAGS}")

message (STATUS "Using C compiler from ${CMAKE_C_COMPILER}")
message (STATUS "Target platform ${TARGET_PLATFORM}")

foreach(lib googletest mylib)
  message (STATUS "Using lib ${lib}")

  add_subdirectory(./libs/${lib})
endforeach(lib)

add_subdirectory(./src)
add_subdirectory(./test)
