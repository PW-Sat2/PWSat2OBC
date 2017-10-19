#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "beacon/sender.hpp"
#include "mock/HasStateMock.hpp"
#include "mock/comm.hpp"
#include "telemetry/state.hpp"

using testing::Return;
using testing::_;
using namespace std::chrono_literals;

namespace
{
    class BeaconSenderTest : public testing::Test
    {
      protected:
        BeaconSenderTest();

        testing::NiceMock<OSMock> _os;
        OSReset _osReset{InstallProxy(&_os)};

        testing::StrictMock<TransmitterMock> _transmitter;
        testing::NiceMock<HasStateMock<telemetry::TelemetryState>> _telemetryState;

        telemetry::TelemetryState _telemetry;

        beacon::BeaconSender _sender{_transmitter, _telemetryState};
    };

    BeaconSenderTest::BeaconSenderTest()
    {
        ON_CALL(_telemetryState, MockGetState()).WillByDefault(testing::ReturnRef(_telemetry));
    }

    TEST_F(BeaconSenderTest, ShouldSendBeaconAndWaitLongDelayOnSuccess)
    {
        EXPECT_CALL(_os, Sleep(60000ms));
        EXPECT_CALL(_transmitter, SendFrame(_)).WillOnce(Return(true));

        _sender.RunOnce();
    }

    TEST_F(BeaconSenderTest, ShouldSendPreviousBeaconAndWaitShortDelayOnFailureToGetTelemetry)
    {
        EXPECT_CALL(_os, Sleep(60000ms));
        EXPECT_CALL(_transmitter, SendFrame(_)).WillOnce(Return(true));
        _sender.RunOnce();

        EXPECT_CALL(_os, TakeSemaphore(_, _)).WillOnce(Return(OSResult::Timeout));
        EXPECT_CALL(_os, Sleep(5000ms));
        EXPECT_CALL(_transmitter, SendFrame(_)).WillOnce(Return(true));
        _sender.RunOnce();
    }

    TEST_F(BeaconSenderTest, ShouldNotSendEmptyFrameAndShouldWaitShortDelayOnFailureToGetTelemetry)
    {
        EXPECT_CALL(_os, TakeSemaphore(_, _)).WillOnce(Return(OSResult::Timeout));
        EXPECT_CALL(_os, Sleep(5000ms));
        EXPECT_CALL(_transmitter, SendFrame(_)).Times(0);
        _sender.RunOnce();
    }

    TEST_F(BeaconSenderTest, ShouldWaitShortDelayOnSendFrameFailure)
    {
        EXPECT_CALL(_os, Sleep(5000ms));
        EXPECT_CALL(_transmitter, SendFrame(_)).WillOnce(Return(false));

        _sender.RunOnce();
    }
}
