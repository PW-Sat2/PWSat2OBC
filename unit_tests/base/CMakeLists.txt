set(NAME unit_tests_base)

set(SOURCES
    assert.cpp
    heap.c
    mem.cpp
    OsMock.cpp
    semihosting.cpp
    test.cpp
    os/os.cpp
    utils.cpp
    mock/AntennaMock.cpp
    mock/comm.cpp
    mock/emlib.cpp
    mock/error_counter.cpp
    mock/FsMock.cpp
    mock/mock.cpp
    mock/error_counter.cpp
    mock/flash_driver.cpp
    mock/InterruptPinDriverMock.cpp
    mock/PayloadHardwareDriverMock.cpp
    mock/SunSDriverMock.cpp
    I2C/I2CMock.cpp
    SPI/SPIMock.cpp
    fault_handlers.c
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
    base
    free_rtos_wrapper
    gtest
    gmock
    platform
    adcs
    comm
    gyro
    eps
    obc_state
    antenna
    mission_comm
    mission_time
    mission_sail
    telecommunication
    program_flash
    payload
    power
    telemetry_imtq
    suns
    payload
    n25q
    experiments
    photo
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mock)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/I2C)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/SPI)

target_compile_options(${NAME} PRIVATE "-fexceptions")

if(${ENABLE_COVERAGE})
  set(LD_COVERAGE "-fprofile-arcs")
  target_link_libraries(${NAME} gcov)
  target_compile_definitions(${NAME} PRIVATE ENABLE_COVERAGE=1)
else()
  set(LD_COVERAGE )    
endif(${ENABLE_COVERAGE})