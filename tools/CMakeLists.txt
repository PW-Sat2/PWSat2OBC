configure_file(${CMAKE_CURRENT_SOURCE_DIR}/upload.cmd.template ${CMAKE_CURRENT_BINARY_DIR}/upload.cmd)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/upload_safe_mode.cmd.template ${CMAKE_CURRENT_BINARY_DIR}/upload_safe_mode.cmd)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/crc.cmd.template ${CMAKE_CURRENT_BINARY_DIR}/crc.cmd)

set(BASE_ADDRESS 0x12000000)
set(LENGTH 0x8000)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/download_mem_range.cmd.template ${CMAKE_CURRENT_BINARY_DIR}/download_safe_mode.cmd)

set(BASE_ADDRESS 0x84000000)
set(LENGTH 0x400000)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/download_mem_range.cmd.template ${CMAKE_CURRENT_BINARY_DIR}/download_program_flash.cmd)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/unit_tests.cmd.template ${CMAKE_CURRENT_BINARY_DIR}/unit_tests.cmd)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pytests.cmd.template ${CMAKE_CURRENT_BINARY_DIR}/pytests.cmd)