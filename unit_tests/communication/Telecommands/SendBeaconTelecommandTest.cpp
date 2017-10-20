#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "mock/HasStateMock.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/comm.hpp"
#include "telemetry/state.hpp"
#include "utils.hpp"

using testing::ReturnRef;
using testing::_;
using testing::Return;
using telecommunication::downlink::DownlinkAPID;

namespace
{
    class SendBeaconTelecommandTest : public testing::Test
    {
      protected:
        testing::NiceMock<OSMock> _os;
        OSReset _osReset{InstallProxy(&_os)};

        testing::NiceMock<HasStateMock<telemetry::TelemetryState>> _stateProvider;
        testing::NiceMock<TransmitterMock> _transmitter;
        obc::telecommands::SendBeaconTelecommand _telecommand{_stateProvider};
    };

    TEST_F(SendBeaconTelecommandTest, ShouldSendBeacon)
    {
        ON_CALL(_os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));

        telemetry::TelemetryState tm;

        ON_CALL(_stateProvider, MockGetState()).WillByDefault(ReturnRef(tm));

        EXPECT_CALL(_transmitter, SendFrame(BeginsWith(telecommunication::downlink::BeaconMarker)));

        _telecommand.Handle(_transmitter, {});
    }

    TEST_F(SendBeaconTelecommandTest, ShouldSendErrorFrameWhenUnableToWriteBeacon)
    {
        ON_CALL(_os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Timeout));

        telemetry::TelemetryState tm;

        ON_CALL(_stateProvider, MockGetState()).WillByDefault(ReturnRef(tm));

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::BeaconError, 0, _)));

        _telecommand.Handle(_transmitter, {});
    }
}
