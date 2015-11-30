set(NAME pwsat)

aux_source_directory(${CMAKE_CURRENT_LIST_DIR} SOURCES)

add_executable(${NAME} ${SOURCES})
target_link_libraries(${NAME} platform)
add_dependencies(${NAME} platform)

target_link_libraries(${NAME} freeRTOS)
add_dependencies(${NAME} freeRTOS)

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
set (HEX_OBJ ${EXEC_OBJ}.hex)

add_custom_command(OUTPUT ${HEX_OBJ}    
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXEC_OBJ} ${HEX_OBJ}
    DEPENDS ${NAME}
)

add_custom_target (hex ALL DEPENDS ${HEX_OBJ})
