set(NAME pwsat)

set(SOURCES main.c)

add_executable(${NAME} ${SOURCES})

target_link_libraries(${NAME} platform)

set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
set (HEX_OBJ ${EXEC_OBJ}.hex)

add_custom_command(OUTPUT ${HEX_OBJ}
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXEC_OBJ} ${HEX_OBJ}
    DEPENDS ${NAME}
)

add_custom_target (hex ALL DEPENDS ${HEX_OBJ})
