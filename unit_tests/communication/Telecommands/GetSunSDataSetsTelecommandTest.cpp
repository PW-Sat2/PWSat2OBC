#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/GyroMock.hpp"
#include "mock/PayloadDeviceMock.hpp"
#include "mock/SunSDriverMock.hpp"
#include "mock/comm.hpp"
#include "mock/power.hpp"
#include "mock/time.hpp"
#include "obc/telecommands/suns.hpp"

using testing::Return;
using testing::ElementsAre;
using testing::_;
using testing::Invoke;
using testing::SetArgReferee;
using testing::DoAll;
using testing::SizeIs;
using testing::InSequence;
using testing::StrEq;

using telecommunication::downlink::DownlinkAPID;
using devices::payload::PayloadTelemetry;
using devices::gyro::GyroscopeTelemetry;
using namespace devices::suns;

using namespace std::literals;

namespace
{
    class GetSunSDataSetsTelecommandTest : public testing::Test
    {
      public:
        GetSunSDataSetsTelecommandTest();

      protected:
        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<PowerControlMock> _power;
        testing::NiceMock<CurrentTimeMock> _timeProvider;
        testing::NiceMock<SunSDriverMock> _sunsExp;
        testing::NiceMock<PayloadDeviceMock> _payload;
        testing::NiceMock<GyroscopeMock> _gyro;

        std::chrono::milliseconds _time{0ms};

        obc::telecommands::GetSunSDataSetsTelecommand _telecommand{//
            _power,                                                //
            _timeProvider,                                         //
            _sunsExp,                                              //
            _payload,                                              //
            _gyro};                                                //

        std::array<uint8_t, 3> _correctParameters;

        static const uint8_t SunSGain = 0x01;
        static const uint8_t SunSITime = 0x02;
    };

    GetSunSDataSetsTelecommandTest::GetSunSDataSetsTelecommandTest()
    {
        ON_CALL(_power, SensPower(_)).WillByDefault(Return(true));
        ON_CALL(_power, SunSPower(_)).WillByDefault(Return(true));

        ON_CALL(_timeProvider, GetCurrentTime()).WillByDefault(Invoke([this]() { return Some(this->_time); }));

        Writer w(_correctParameters);
        w.WriteByte(0xFF);
        w.WriteByte(SunSGain);
        w.WriteByte(SunSITime);
    }

    TEST_F(GetSunSDataSetsTelecommandTest, ShouldCallAllDependencies)
    {
        MeasurementData expSunsData;
        PayloadTelemetry::SunsRef refSunsData;
        GyroscopeTelemetry gyroData;

        EXPECT_CALL(_power, SensPower(true));
        EXPECT_CALL(_power, SunSPower(true));

        EXPECT_CALL(_power, SensPower(false));
        EXPECT_CALL(_power, SunSPower(false));

        EXPECT_CALL(_timeProvider, GetCurrentTime());

        EXPECT_CALL(_sunsExp, StartMeasurement(SunSGain, SunSITime)).WillOnce(Return(OperationStatus::OK));
        EXPECT_CALL(_payload, MeasureSunSRef(_)).WillOnce(DoAll(SetArgReferee<0>(refSunsData), Return(OSResult::Success)));
        EXPECT_CALL(_sunsExp, WaitForData()).WillOnce(Return(OSResult::Success));
        EXPECT_CALL(_sunsExp, GetMeasuredData(_)).WillOnce(DoAll(SetArgReferee<0>(expSunsData), Return(OperationStatus::OK)));
        EXPECT_CALL(_gyro, read()).WillOnce(Return(Some(gyroData)));

        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::SunS, 0, _)));

        _telecommand.Handle(_transmitter, _correctParameters);
    }

    TEST_F(GetSunSDataSetsTelecommandTest, ShouldTransmitDownlinkOperationFrame)
    {
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::SunS, 0, _)));

        _telecommand.Handle(_transmitter, _correctParameters);
    }

    TEST_F(GetSunSDataSetsTelecommandTest, ShouldPowerOnSunSBeforeStartingMeasurement)
    {
        MeasurementData expSunsData;

        {
            InSequence s;

            EXPECT_CALL(_power, SunSPower(true));

            EXPECT_CALL(_sunsExp, StartMeasurement(_, _)).WillOnce(Return(OperationStatus::OK));
            EXPECT_CALL(_sunsExp, WaitForData()).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(_sunsExp, GetMeasuredData(_)).WillOnce(DoAll(SetArgReferee<0>(expSunsData), Return(OperationStatus::OK)));

            EXPECT_CALL(_power, SunSPower(false));
        }

        _telecommand.Handle(_transmitter, _correctParameters);
    }

    TEST_F(GetSunSDataSetsTelecommandTest, ShouldPowerOnPayloadBeforeMeasuringSunSRef)
    {
        PayloadTelemetry::SunsRef refSunsData;

        {
            InSequence s;

            EXPECT_CALL(_power, SensPower(true));

            EXPECT_CALL(_payload, MeasureSunSRef(_)).WillOnce(DoAll(SetArgReferee<0>(refSunsData), Return(OSResult::Success)));

            EXPECT_CALL(_power, SensPower(false));
        }

        _telecommand.Handle(_transmitter, _correctParameters);
    }

    TEST_F(GetSunSDataSetsTelecommandTest, ShouldNotRunIfParametersAreIncorrect)
    {
        std::array<uint8_t, 1> incorrectParameters;
        Writer w(incorrectParameters);
        w.WriteByte(0xAB);

        EXPECT_CALL(_power, SensPower(_)).Times(0);
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::SunS, 0, ElementsAre(0xAB, 0x01))));

        _telecommand.Handle(_transmitter, incorrectParameters);
    }
}
