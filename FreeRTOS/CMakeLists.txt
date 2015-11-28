cmake_minimum_required (VERSION 2.6)

set(LOCAL_HEADERS ${CMAKE_CURRENT_LIST_DIR}/${INCLUDE_DIR}/)
set(EXT_DIR ${CMAKE_CURRENT_LIST_DIR}/Portable/Portable-${ARCH}/${CHIP}/)
set(LOCAL_TARGET freeRTOS)
if (${MEM_MANAGMENT_TYPE} GREATER 0)
    set(MEM_MANG  ${CMAKE_CURRENT_LIST_DIR}/Portable/MemMang/heap_${MEM_MANAGMENT_TYPE}.c)
else ()
    message(FATAL_ERROR "You didn't set MEM_MANAGMENT_TYPE!")
endif()
# install headers
file(COPY ${LOCAL_HEADERS} DESTINATION ${DEST_INCLUDE_DIR})
file(COPY ${EXT_DIR} DESTINATION ${DEST_INCLUDE_DIR})

include_directories (${DEST_INCLUDE_DIR})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/${SOURCE_DIR} FREERTOS_SOURCE_PATH)
aux_source_directory(${EXT_DIR} FREERTOS_SOURCE_PATH)

add_library(${LOCAL_TARGET} STATIC ${FREERTOS_SOURCE_PATH} ${MEM_MANG})

target_link_libraries(${LOCAL_TARGET} platform)
add_dependencies(${LOCAL_TARGET} platform)
