#ifndef MOCK_IMTQ_DRIVER_HPP
#define MOCK_IMTQ_DRIVER_HPP

#include <chrono>

class ImtqDriverMock : public devices::imtq::IImtqDriver
{
  public:
    ImtqDriverMock() = default;
    ~ImtqDriverMock() = default;

    MOCK_METHOD1(PerformSelfTest, bool(devices::imtq::SelfTestResult&));
    MOCK_METHOD1(MeasureMagnetometer, bool(devices::imtq::Vector3<devices::imtq::MagnetometerMeasurement>&));
    MOCK_METHOD0(SoftwareReset, bool());
    MOCK_METHOD0(SendNoOperation, bool());
    MOCK_METHOD0(CancelOperation, bool());
    MOCK_METHOD0(StartMTMMeasurement, bool());
    MOCK_METHOD2(StartActuationCurrent, bool(const devices::imtq::Vector3<devices::imtq::Current>&, std::chrono::milliseconds));
    MOCK_METHOD2(StartActuationDipole, bool(devices::imtq::Vector3<devices::imtq::Dipole>, std::chrono::milliseconds));
    MOCK_METHOD0(StartAllAxisSelfTest, bool());
    MOCK_METHOD1(StartBDotDetumbling, bool(std::chrono::seconds));
    MOCK_METHOD1(GetSystemState, bool(devices::imtq::State&));
    MOCK_METHOD1(GetCalibratedMagnetometerData, bool(devices::imtq::MagnetometerMeasurementResult&));
    MOCK_METHOD1(GetCoilCurrent, bool(devices::imtq::Vector3<devices::imtq::Current>&));
    MOCK_METHOD1(GetCoilTemperature, bool(devices::imtq::Vector3<devices::imtq::TemperatureMeasurement>&));
    MOCK_METHOD1(GetSelfTestResult, bool(devices::imtq::SelfTestResult&));
    MOCK_METHOD1(GetDetumbleData, bool(devices::imtq::DetumbleData&));
    MOCK_METHOD1(GetHouseKeepingRAW, bool(devices::imtq::HouseKeepingRAW&));
    MOCK_METHOD1(GetHouseKeepingEngineering, bool(devices::imtq::HouseKeepingEngineering&));
    MOCK_METHOD2(GetParameter, bool(Parameter, gsl::span<std::uint8_t>));
    MOCK_METHOD2(SetParameter, bool(Parameter, gsl::span<const std::uint8_t>));
    MOCK_METHOD2(ResetParameterAndGetDefault, bool(Parameter, gsl::span<std::uint8_t>));
};

#endif /* MOCK_IMTQ_DRIVER_HPP */
