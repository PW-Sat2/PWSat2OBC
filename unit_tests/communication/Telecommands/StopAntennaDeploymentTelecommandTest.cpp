#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/comm.hpp"
#include "obc/telecommands/antenna.hpp"

using testing::_;
using testing::Eq;
using testing::Return;
using testing::ReturnRef;
using testing::ElementsAre;
using telecommunication::downlink::DownlinkAPID;

namespace
{
    class DisableAntennaDeploymentMock : public mission::antenna::IDisableAntennaDeployment
    {
      public:
        MOCK_METHOD1(SetDeploymentState, void(bool disabled));
    };

    class StopAntennaDeploymentTelecommandTest : public testing::Test
    {
      protected:
        StopAntennaDeploymentTelecommandTest();

        template <typename... T> void Run(T... params);

        void ExpectFrame(std::uint8_t correlationId, std::uint8_t errorCode);

        void ExpectUpdate(bool disabled);

        testing::StrictMock<TransmitterMock> _transmitter;
        testing::StrictMock<DisableAntennaDeploymentMock> _controller;

        SystemState _state;
        obc::telecommands::SetAntennaDeploymentMaskTelecommand _telecommand;
    };

    StopAntennaDeploymentTelecommandTest::StopAntennaDeploymentTelecommandTest() : _telecommand(_controller)
    {
    }

    template <typename... T> void StopAntennaDeploymentTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(_transmitter, buffer);
    }

    void StopAntennaDeploymentTelecommandTest::ExpectFrame(std::uint8_t correlationId, std::uint8_t errorCode)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(correlationId, errorCode))));
    }

    void StopAntennaDeploymentTelecommandTest::ExpectUpdate(bool disabled)
    {
        EXPECT_CALL(_controller, SetDeploymentState(disabled)).Times(1);
    }

    TEST_F(StopAntennaDeploymentTelecommandTest, TestEmptyFrame)
    {
        ExpectFrame(0, -1);
        Run();
    }

    TEST_F(StopAntennaDeploymentTelecommandTest, TestPartialFrame)
    {
        ExpectFrame(0x22, -1);
        Run(0x22);
    }

    TEST_F(StopAntennaDeploymentTelecommandTest, TestDisablingDeploymentFrame)
    {
        ExpectFrame(0x22, 0);
        ExpectUpdate(true);
        Run(0x22, 1);
    }

    TEST_F(StopAntennaDeploymentTelecommandTest, TestEnablingDeploymentFrame)
    {
        ExpectFrame(0x22, 0);
        ExpectUpdate(false);
        Run(0x22, 0);
    }
}
