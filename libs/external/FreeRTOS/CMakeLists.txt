set(NAME freeRTOS)

set(PORT_DIR ${CMAKE_CURRENT_LIST_DIR}/Portable/Portable-${ARCH}/${CHIP}/)

if (${MEM_MANAGMENT_TYPE} GREATER 0)
    set(MEM_MANG  ${CMAKE_CURRENT_LIST_DIR}/Portable/MemMang/heap_${MEM_MANAGMENT_TYPE}.c)
else ()
    message(FATAL_ERROR "You didn't set MEM_MANAGMENT_TYPE!")
endif()

aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Source SOURCES)
aux_source_directory(${PORT_DIR} PORT_SOURCES)

add_library(${NAME} STATIC ${SOURCES} ${MEM_MANG} ${PORT_SOURCES})

target_include_directories(${NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR}/Include ${PORT_DIR})

target_link_libraries(${NAME} 
	platform 
	)
