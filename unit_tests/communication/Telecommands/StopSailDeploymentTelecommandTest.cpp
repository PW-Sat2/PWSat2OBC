#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mission/sail.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/sail.hpp"

using testing::Eq;
using testing::ElementsAre;
using telecommunication::downlink::DownlinkAPID;

namespace
{
    class DisableSailDeploymentMock : public mission::IDisableSailDeployment
    {
      public:
        MOCK_METHOD0(DisableDeployment, void());
    };

    class StopSailDeploymentTelecommandTest : public testing::Test
    {
      protected:
        testing::NiceMock<DisableSailDeploymentMock> disableDeployment;
        testing::NiceMock<TransmitterMock> transmitter;

        obc::telecommands::StopSailDeployment telecommand{disableDeployment};
    };

    TEST_F(StopSailDeploymentTelecommandTest, ShouldDisableDeployment)
    {
        EXPECT_CALL(this->disableDeployment, DisableDeployment());

        EXPECT_CALL(this->transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::DisableSailDeployment), Eq(0U), ElementsAre(0x22, 0))));

        std::array<std::uint8_t, 1> frame{0x22};

        telecommand.Handle(this->transmitter, frame);
    }

    TEST_F(StopSailDeploymentTelecommandTest, ShouldRespondWithErrorOnInvalidFrame)
    {
        EXPECT_CALL(this->disableDeployment, DisableDeployment()).Times(0);

        EXPECT_CALL(this->transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::DisableSailDeployment), Eq(0U), ElementsAre(0, 255))));

        telecommand.Handle(this->transmitter, gsl::span<const uint8_t>());
    }
}
