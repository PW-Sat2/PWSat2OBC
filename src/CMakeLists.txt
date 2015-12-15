set(NAME pwsat)

set(SOURCES main.c)

add_executable(${NAME} ${SOURCES})

target_link_libraries(${NAME} platform)

target_generate_hex(${NAME})
