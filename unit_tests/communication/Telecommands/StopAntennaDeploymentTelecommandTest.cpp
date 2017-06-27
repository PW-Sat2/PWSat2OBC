#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/comm.hpp"
#include "obc/telecommands/antenna.hpp"

using testing::Eq;
using testing::ElementsAre;
using telecommunication::downlink::DownlinkAPID;

namespace
{
    class DisableAntennaDeploymentMock : public mission::antenna::IDisableAntennaDeployment
    {
      public:
        MOCK_METHOD0(DisableDeployment, void());
    };

    class StopAntennaDeploymentTelecommandTest : public testing::Test
    {
      protected:
        StopAntennaDeploymentTelecommandTest();

        testing::NiceMock<DisableAntennaDeploymentMock> _disableDeployment;
        testing::NiceMock<TransmitterMock> _transmitter;

        obc::telecommands::StopAntennaDeployment _telecommand;
    };

    StopAntennaDeploymentTelecommandTest::StopAntennaDeploymentTelecommandTest() : _telecommand(_disableDeployment)
    {
    }

    TEST_F(StopAntennaDeploymentTelecommandTest, ShouldDisableDeployment)
    {
        EXPECT_CALL(this->_disableDeployment, DisableDeployment());

        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(0U), ElementsAre(0x22, 0))));

        std::array<std::uint8_t, 1> frame{0x22};

        _telecommand.Handle(this->_transmitter, frame);
    }

    TEST_F(StopAntennaDeploymentTelecommandTest, ShouldRespondWithErrorOnInvalidFrame)
    {
        EXPECT_CALL(this->_disableDeployment, DisableDeployment()).Times(0);

        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(Eq(DownlinkAPID::Operation), Eq(0U), ElementsAre(0, 255))));

        _telecommand.Handle(this->_transmitter, gsl::span<const uint8_t>());
    }
}
