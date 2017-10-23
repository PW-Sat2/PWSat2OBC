#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "mission/sail.hpp"
#include "mock/HasStateMock.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/sail.hpp"
#include "os/os.hpp"

using telecommunication::downlink::DownlinkAPID;
using testing::ElementsAre;
using testing::Eq;
using testing::Return;
using testing::ReturnRef;
using testing::_;

namespace
{
    class StopSailDeploymentTelecommandTest : public testing::Test
    {
      protected:
        StopSailDeploymentTelecommandTest();
        testing::NiceMock<HasStateMock<SystemState>> stateContainer;
        testing::NiceMock<TransmitterMock> transmitter;

        testing::NiceMock<OSMock> os;
        OSReset guard;

        obc::telecommands::StopSailDeployment telecommand{stateContainer};
    };

    StopSailDeploymentTelecommandTest::StopSailDeploymentTelecommandTest()
    {
        this->guard = InstallProxy(&os);
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
    }

    TEST_F(StopSailDeploymentTelecommandTest, ShouldDisableDeploymentBeforeItBegins)
    {
        SystemState state;
        auto& persistentState = state.PersistentState;
        persistentState.Set(state::SailState(state::SailOpeningState::Waiting));
        EXPECT_CALL(stateContainer, MockGetState()).WillOnce(ReturnRef(state));

        EXPECT_CALL(this->transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::DisableSailDeployment), Eq(0U), ElementsAre(0x22, 0))));

        std::array<std::uint8_t, 1> frame{0x22};

        telecommand.Handle(this->transmitter, frame);

        state::SailState sailState;
        persistentState.Get(sailState);
        ASSERT_THAT(sailState.CurrentState(), Eq(state::SailOpeningState::OpeningStopped));
    }

    TEST_F(StopSailDeploymentTelecommandTest, ShouldStopOpening)
    {
        SystemState state;
        auto& persistentState = state.PersistentState;
        persistentState.Set(state::SailState(state::SailOpeningState::Opening));
        EXPECT_CALL(stateContainer, MockGetState()).WillOnce(ReturnRef(state));

        EXPECT_CALL(this->transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::DisableSailDeployment), Eq(0U), ElementsAre(0x22, 0))));

        std::array<std::uint8_t, 1> frame{0x22};

        telecommand.Handle(this->transmitter, frame);

        state::SailState sailState;
        persistentState.Get(sailState);
        ASSERT_THAT(sailState.CurrentState(), Eq(state::SailOpeningState::OpeningStopped));
    }

    TEST_F(StopSailDeploymentTelecommandTest, ShouldRespondWithErrorOnInvalidFrame)
    {
        SystemState state;
        ON_CALL(stateContainer, MockGetState()).WillByDefault(ReturnRef(state));
        EXPECT_CALL(this->transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::DisableSailDeployment), Eq(0U), ElementsAre(0, 255))));

        telecommand.Handle(this->transmitter, gsl::span<const uint8_t>());
    }
}
