cmake_minimum_required (VERSION 2.6)

set(LOCAL_HEADERS ${CMAKE_CURRENT_LIST_DIR}/${INCLUDE_DIR}/)
set(LOCAL_TARGET common)

# install headers
file(COPY ${LOCAL_HEADERS} DESTINATION ${DEST_INCLUDE_DIR})

include_directories (${DEST_INCLUDE_DIR})

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Minimal COMMON_SOURCE_PATH)

add_library(${LOCAL_TARGET} STATIC ${COMMON_SOURCE_PATH})

target_link_libraries(${LOCAL_TARGET} freeRTOS)
add_dependencies(${LOCAL_TARGET} freeRTOS)
