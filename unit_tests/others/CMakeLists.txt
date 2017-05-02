set(NAME unit_tests_others)

set(SOURCES
  adcs/AdcsCoordinatorTest.cpp    
  Logger/LoggerTest.cpp
  FileSystem/FileSystemTest.cpp
  FileSystem/YaffsOSGlue.cpp
  FileSystem/MemoryDriver.cpp
  FileSystem/NANDGeometryTest.cpp
  FileSystem/MemoryDriver.cpp
  FileSystem/EccTest.cpp
  FileSystem/FileTest.cpp
  base/ReaderTest.cpp
  base/WriterTest.cpp
  base/OnLeaveTest.cpp
  base/RedundancyTest.cpp
  base/CRCTest.cpp
  os/TimeoutTest.cpp
  os/EventGroupTest.cpp
  time/time.cpp
  time/timer.cpp
  time/SmartWait.cpp
  eigen/eigenTest.cpp  
  ErrorCounterTest.cpp    
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME}
    yaffs
    m
    platform
    spi
    emlib
    logger
    storage
    comm
    telecommunication
    fs    
    base       
    emdrv
    time
    gsl
    antenna    
    eigen
    exp_fibo
    adcs
    obc_telecommands    
    eps
    rtc
    error_counter
    fm25w
    unit_tests_base
)


