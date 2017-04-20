function(target_generate_hex TARGET)
  set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET})
  set (HEX_OBJ ${EXEC_OBJ}.hex)

  set_target_properties(${TARGET} PROPERTIES HEX_FILE ${HEX_OBJ})

  add_custom_command(OUTPUT ${HEX_OBJ}
      COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXEC_OBJ} ${HEX_OBJ}
      DEPENDS ${TARGET}
  )

  add_custom_target (${TARGET}.hex ALL DEPENDS ${HEX_OBJ})
endfunction(target_generate_hex)

function(target_generate_bin TARGET)
  set (EXEC_OBJ ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET})
  set (BIN_OBJ ${EXEC_OBJ}.bin)

  set_target_properties(${TARGET} PROPERTIES BIN_FILE ${BIN_OBJ})

  add_custom_command(OUTPUT ${BIN_OBJ}
      COMMAND ${CMAKE_OBJCOPY} -O binary ${EXEC_OBJ} ${BIN_OBJ}
      DEPENDS ${TARGET}
  )

  add_custom_target (${TARGET}.bin ALL DEPENDS ${BIN_OBJ})
endfunction(target_generate_bin)


function(target_jlink_flash TARGET BASE_ADDRESS)
  set(COMMAND_FILE ${CMAKE_BINARY_DIR}/jlink/${TARGET}.flash.jlink)

  get_property(HEX_FILE TARGET ${TARGET} PROPERTY HEX_FILE)

  configure_file(${CMAKE_SOURCE_DIR}/jlink/flash.jlink.template ${COMMAND_FILE})

  unset(HEX_FILE)

  set(JLINK_ARGS 
      "-device" ${DEVICE}
      "-ExitOnError"
      "-CommanderScript" ${COMMAND_FILE}
  )

  if(NOT ${JLINK_SN} STREQUAL "")
      list(APPEND JLINK_ARGS -SelectEmuBySN ${JLINK_SN})
  endif()
  
  add_custom_target(${TARGET}.flash    
    COMMAND ${JLINK} ${JLINK_ARGS}
    DEPENDS ${TARGET}.hex
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )

endfunction(target_jlink_flash)

function(target_format_sources TARGET SOURCES)
    if(CLANG_FORMAT)
        add_custom_target(${NAME}.format
            COMMAND ${CLANG_FORMAT} -style=file -i ${SOURCES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Formatting files: '${SOURCES}'"
            )
    else(CLANG_FORMAT)
        add_custom_target(${NAME}.format
            COMMAND ""
            COMMENT "Clang format has not been found. Code formatting is not available. "
            )
    endif()
endfunction(target_format_sources)

function(target_memory_report TARGET)
    get_property(binary TARGET ${TARGET} PROPERTY RUNTIME_OUTPUT_NAME)

    add_custom_target(${TARGET}.memory_report
        COMMAND ${CMAKE_GCC_SIZE} -A -d $<TARGET_FILE:${TARGET}> > ${REPORT_PATH}/${TARGET}.size
        COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/utils/memory_report.py ${OUTPUT_PATH}/bin/${TARGET}.map ${REPORTS_PATH}/memory
        DEPENDS ${TARGET} utils.deps
        )
    set_property(TARGET ${NAME} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-Map=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME}.map")
endfunction(target_memory_report)

function(target_asm_listing TARGET)
    get_property(binary TARGET ${TARGET} PROPERTY RUNTIME_OUTPUT_NAME)

    add_custom_target(${TARGET}.asm
        COMMAND ${CMAKE_COMMAND} -E make_directory ${REPORTS_PATH}
        COMMAND ${CMAKE_OBJDUMP} -dSC $<TARGET_FILE:${TARGET}> > ${REPORTS_PATH}/${TARGET}.lss
        DEPENDS ${TARGET}
    )
endfunction(target_asm_listing)

function(generate_version_file FILENAME)
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD 
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_COMMIT_HASH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      OUTPUT_VARIABLE GIT_BRANCH
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    
    cmake_host_system_information(RESULT HOST_NAME QUERY HOSTNAME)
    
    configure_file(${CMAKE_SOURCE_DIR}/utils/version.h.template ${FILENAME})
endfunction(generate_version_file)

function(target_eclipse_debug_configs NAME)
    get_filename_component(BUILD_DIR_NAME ${CMAKE_BINARY_DIR} NAME) 

    foreach(target ${ARGN})
        configure_file(${CMAKE_SOURCE_DIR}/utils/${target}.launch.template "${CMAKE_CURRENT_BINARY_DIR}/[${NAME}] ${target}.launch")
    endforeach(target)

endfunction(target_eclipse_debug_configs)