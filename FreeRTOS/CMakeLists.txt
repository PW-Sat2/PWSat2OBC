cmake_minimum_required (VERSION 2.8)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(LOCAL_HEADERS ${CMAKE_CURRENT_LIST_DIR}/${INCLUDE_DIR}/)
set(EXT_DIR ${CMAKE_CURRENT_LIST_DIR}/Portable/Portable-${ARCH}/${CHIP}/)
set(LOCAL_TARGET freeRTOS)

if (UNIX)
    add_definitions ("-DWall")
else ()
    message (STATUS "Unrecognized system.")
endif()

# install headers
file(COPY ${LOCAL_HEADERS} DESTINATION ${DEST_INCLUDE_DIR})
file(COPY ${EXT_DIR} DESTINATION ${DEST_INCLUDE_DIR})

include_directories (${DEST_INCLUDE_DIR})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/${SOURCE_DIR} FREERTOS_SOURCE_PATH)
aux_source_directory(${EXT_DIR} FREERTOS_SOURCE_PATH)

add_library(${LOCAL_TARGET} OBJECT ${FREERTOS_SOURCE_PATH})

#target_link_libraries(${LOCAL_TARGET} platform)
add_dependencies(${LOCAL_TARGET} platform)
