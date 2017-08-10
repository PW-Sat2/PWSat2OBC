#ifndef MOCK_SUNS_DRIVER_HPP
#define MOCK_SUNS_DRIVER_HPP

#include "gmock/gmock.h"
#include "suns/suns.hpp"

class SunSDriverMock : public devices::suns::ISunSDriver
{
  public:
    SunSDriverMock();
    ~SunSDriverMock();

    MOCK_METHOD3(MeasureSunS, devices::suns::OperationStatus(devices::suns::MeasurementData&, uint8_t, uint8_t));
    MOCK_METHOD2(StartMeasurement, devices::suns::OperationStatus(uint8_t, uint8_t));
    MOCK_METHOD1(GetMeasuredData, devices::suns::OperationStatus(devices::suns::MeasurementData&));

    MOCK_CONST_METHOD0(IsBusy, bool());
    MOCK_METHOD0(WaitForData, OSResult());
    MOCK_METHOD0(RaiseDataReadyISR, void());
    MOCK_METHOD1(SetDataTimeout, void(std::chrono::milliseconds newTimeout));
};

#endif /* MOCK_SUNS_DRIVER_HPP */
