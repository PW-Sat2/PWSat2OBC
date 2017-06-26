set(NAME unit_tests_drivers)

set(SOURCES
  antenna/MiniportMock.cpp
  antenna/MiniportMock.hpp
  antenna/miniport.cpp
  antenna/antenna.cpp
  Comm/CommTest.cpp
  Comm/CommFrameTest.cpp
  Comm/DownlinkFrameTest.cpp
  Comm/CommTelemetryTest.cpp
  Comm/UplinkFrameDecoderTest.cpp
  EPS/EPSDriverTest.cpp
  EPS/EpsTelemetryTest.cpp
  SPI/SPIDriverTest.cpp
  FM25W/FM25WDriverTest.cpp
  FM25W/RedundantFM25WDriverTest.cpp
  I2C/FallbackI2CBusTest.cpp
  I2C/ErrorHandlingI2CBusTest.cpp
  N25Q/N25QTest.cpp
  N25Q/RedundantN25QTest.cpp
  imtq/imtqTest.cpp
  imtq/imtqHighLevelTest.cpp
  RTC/RTCTest.cpp
  gyro/gyroTest.cpp
  gyro/GyroTelemetryTest.cpp
  payload/PayloadDeviceDriverTest.cpp
)

add_unit_tests(${NAME} ${SOURCES})

target_link_libraries(${NAME}
    telecommunication    
    eps    
    spi
    fm25w
    n25q
    imtq
    rtc
    unit_tests_base
    gyro
    payload
)

