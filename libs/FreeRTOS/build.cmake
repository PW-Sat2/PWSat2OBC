set(NAME freeRTOS)

set(LOCAL_HEADERS ${CMAKE_CURRENT_LIST_DIR}/Include)
set(EXT_DIR ${CMAKE_CURRENT_LIST_DIR}/Portable/Portable-${ARCH}/${CHIP}/)

if (${MEM_MANAGMENT_TYPE} GREATER 0)
    set(MEM_MANG  ${CMAKE_CURRENT_LIST_DIR}/Portable/MemMang/heap_${MEM_MANAGMENT_TYPE}.c)
else ()
    message(FATAL_ERROR "You didn't set MEM_MANAGMENT_TYPE!")
endif()

include_directories(${LOCAL_HEADERS})
include_directories(${EXT_DIR})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Source FREERTOS_SOURCE_PATH)
aux_source_directory(${EXT_DIR} FREERTOS_SOURCE_PATH)

add_library(${NAME} STATIC ${FREERTOS_SOURCE_PATH} ${MEM_MANG})

target_link_libraries(${NAME} platform)
add_dependencies(${NAME} platform)
