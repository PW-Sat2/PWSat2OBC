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
    I2C/I2CMock.cpp
    SPI/SPIMock.cpp
)

add_library(${NAME} STATIC ${SOURCES})

target_link_libraries(${NAME} 
	base
	gtest
    gmock
    platform
    adcs
    comm
    obc_state
    antenna
)

target_include_directories(${NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/mock)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/I2C)
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include/SPI)

target_compile_options(${NAME} PRIVATE "-fexceptions")