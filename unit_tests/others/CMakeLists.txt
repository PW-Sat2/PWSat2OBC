set(NAME unit_tests_others)

add_subdirectory(adcs)

set(SOURCES
  adcs/AdcsCoordinatorTest.cpp
  adcs/experimental/detumblingTest.cpp  
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
  base/BitWriterTest.cpp
  base/CounterTest.cpp
  os/TimeoutTest.cpp
  os/EventGroupTest.cpp
  time/time.cpp
  time/timer.cpp
  time/SmartWait.cpp
  eigen/eigenTest.cpp  
  ErrorCounterTest.cpp    
  BootSettings/BootSettingsTest.cpp
  Scrubbing/shared.cpp
  Scrubbing/ProgramScrubbingTest.cpp
  Scrubbing/BootloaderScrubbingTest.cpp
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
    adcs_experimental
    obc_telecommands    
    eps
    rtc
    error_counter
    fm25w
    unit_tests_base
    boot_settings
    scrubber
)


