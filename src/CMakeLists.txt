set(NAME pwsat)

set(SOURCES main.c)

add_executable(${NAME} ${SOURCES})

target_link_libraries(${NAME} platform freeRTOS)

target_generate_hex(${NAME})

target_jlink_flash(${NAME} "${CMAKE_CURRENT_SOURCE_DIR}/../jlink/flash.jlink")
