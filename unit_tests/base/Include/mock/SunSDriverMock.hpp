#ifndef MOCK_SUNS_DRIVER_HPP
#define MOCK_SUNS_DRIVER_HPP

class SunSDriverMock : public devices::suns::ISunSDriver
{
  public:
    SunSDriverMock() = default;
    ~SunSDriverMock() = default;

    MOCK_METHOD3(MeasureSunS, devices::suns::OperationStatus(devices::suns::MeasurementData&, uint8_t, uint8_t));
    MOCK_METHOD2(StartMeasurement, devices::suns::OperationStatus(uint8_t, uint8_t));
    MOCK_METHOD1(GetMeasuredData, devices::suns::OperationStatus(devices::suns::MeasurementData&));
};

#endif /* MOCK_SUNS_DRIVER_HPP */
