#ifndef UNIT_TESTS_BASE_INCLUDE_MOCK_TEMPERATUREREADERMOCK_HPP_
#define UNIT_TESTS_BASE_INCLUDE_MOCK_TEMPERATUREREADERMOCK_HPP_

#include "gmock/gmock.h"
#include "temp/mcu.hpp"

class TemperatureReaderMock : public temp::ITemperatureReader
{
  public:
    TemperatureReaderMock();
    ~TemperatureReaderMock();

    MOCK_METHOD0(ReadCelsius, std::int16_t());
    MOCK_METHOD0(ReadRaw, BitValue<std::uint16_t, 12>());
};

#endif /* UNIT_TESTS_BASE_INCLUDE_MOCK_TEMPERATUREREADERMOCK_HPP_ */
