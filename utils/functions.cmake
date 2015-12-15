function(target_generate_hex TARGET)
  set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET})
  set (HEX_OBJ ${EXEC_OBJ}.hex)

  add_custom_command(OUTPUT ${HEX_OBJ}
      COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXEC_OBJ} ${HEX_OBJ}
      DEPENDS ${TARGET}
  )

  add_custom_target (${TARGET}.hex ALL DEPENDS ${HEX_OBJ})
endfunction(target_generate_hex)

function(target_jlink_flash TARGET COMMAND_FILE)
  add_custom_target(${TARGET}.flash
    COMMAND ${JLINK} -device ${DEVICE} -ExitOnError -CommanderScript ${COMMAND_FILE}
    DEPENDS ${TARGET}.hex
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )

endfunction(target_jlink_flash)
