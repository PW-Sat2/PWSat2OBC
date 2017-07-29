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
using testing::Return;
using testing::ReturnRef;
using testing::ElementsAre;
using telecommunication::downlink::DownlinkAPID;
using namespace std::chrono_literals;

namespace
{
    class SetPeriodicMessageTelecommandTest : public testing::Test
    {
      protected:
        SetPeriodicMessageTelecommandTest();

        template <typename... T> void Run(T... params);

        testing::NiceMock<OSMock> _os;
        OSReset _osReset{InstallProxy(&_os)};

        testing::NiceMock<HasStateMock<SystemState>> _stateContainer;

        testing::NiceMock<TransmitterMock> _transmitter;

        obc::telecommands::SetPeriodicMessageTelecommand _telecommand{_stateContainer};
    };

    SetPeriodicMessageTelecommandTest::SetPeriodicMessageTelecommandTest()
    {
        ON_CALL(_os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
    }

    template <typename... T> void SetPeriodicMessageTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(_transmitter, buffer);
    }

    TEST_F(SetPeriodicMessageTelecommandTest, ShouldSetNewPeriodicMessage)
    {
        SystemState state;

        ON_CALL(_stateContainer, MockGetState()).WillByDefault(ReturnRef(state));

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x13, 0x0))));

        Run(0x13, 3, 4, 'T', 'e', 's', 't');

        state::MessageState settings;
        state.PersistentState.Get(settings);

        ASSERT_THAT(settings.Interval(), Eq(3min));
        ASSERT_THAT(settings.RepeatCount(), Eq(4));
        ASSERT_THAT(settings.Message().subspan(0, 4), ElementsAre('T', 'e', 's', 't'));
    }

    TEST_F(SetPeriodicMessageTelecommandTest, ShouldRespondWithErrorOnEmptyFrame)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x0, 0x1))));

        Run();
    }

    TEST_F(SetPeriodicMessageTelecommandTest, ShouldRespondWithErrorOnTooShortFrame)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x13, 0x1))));

        Run(0x13, 0x1, 0x2);
    }
}
