#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "mock/HasStateMock.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/periodic_message.hpp"
#include "state/struct.h"

using testing::_;
using testing::Eq;
using testing::ContainerEq;
using testing::ReturnRef;
using testing::Return;
using namespace std::chrono_literals;
using telecommunication::downlink::DownlinkAPID;

namespace
{
    class SendPeriodicMessageTelecommandTest : public testing::Test
    {
      protected:
        testing::NiceMock<OSMock> _os;
        OSReset _osReset{InstallProxy(&_os)};

        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<HasStateMock<SystemState>> _stateContainer;

        obc::telecommands::SendPeriodicMessageTelecommand _telecommand{_stateContainer};
    };

    TEST_F(SendPeriodicMessageTelecommandTest, ShouldSend40PeriodicMessagesIfNoParametersGiven)
    {
        SystemState state;

        std::array<std::uint8_t, state::MessageState::MessageLength> message{65, 66, 67, 68};

        state.PersistentState.Set(state::MessageState(1min, 2, message));

        ON_CALL(this->_os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));

        ON_CALL(_stateContainer, MockGetState()).WillByDefault(ReturnRef(state));

        auto s = gsl::make_span(message);

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::PeriodicMessage, 0, s))).Times(40);

        _telecommand.Handle(_transmitter, {});
    }

    TEST_F(SendPeriodicMessageTelecommandTest, ShouldSendRequestedNumberOfPeriodicMessages)
    {
        SystemState state;

        std::array<std::uint8_t, state::MessageState::MessageLength> message{65, 66, 67, 68};

        state.PersistentState.Set(state::MessageState(1min, 2, message));

        ON_CALL(this->_os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));

        ON_CALL(_stateContainer, MockGetState()).WillByDefault(ReturnRef(state));

        auto s = gsl::make_span(message);

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::PeriodicMessage, 0, s))).Times(15);

        std::uint8_t params[] = {15};

        _telecommand.Handle(_transmitter, params);
    }
}
