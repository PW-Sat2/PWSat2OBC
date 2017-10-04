#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/AdcsMocks.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/adcs.hpp"

using telecommunication::downlink::DownlinkAPID;
using testing::ElementsAre;
using testing::Return;

namespace
{
    class SetAdcsModeExperimentTelecommandTest : public testing::Test
    {
      protected:
        template <typename... T> void Run(T... params);

        void ExpectFrame(std::uint8_t correlationId, std::uint8_t errorCode);

        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<AdcsCoordinatorMock> _coordinator;

        obc::telecommands::SetAdcsModeTelecommand _telecommand{_coordinator};
    };

    template <typename... T> void SetAdcsModeExperimentTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(_transmitter, buffer);
    }

    void SetAdcsModeExperimentTelecommandTest::ExpectFrame(std::uint8_t correlationId, std::uint8_t errorCode)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::SetAdcsMode, 0, ElementsAre(correlationId, errorCode))));
    }

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestStoppingAdcs)
    {
        EXPECT_CALL(_coordinator, Stop());

        ExpectFrame(10, 0);

        Run(10, adcs::AdcsMode::Stopped);
    }

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestDisablingAdcs)
    {
        EXPECT_CALL(_coordinator, Disable());

        ExpectFrame(11, 0);

        Run(11, adcs::AdcsMode::Disabled);
    }

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestBuiltinDetumbling)
    {
        EXPECT_CALL(_coordinator, EnableBuiltinDetumbling());

        ExpectFrame(11, 0);

        Run(11, adcs::AdcsMode::BuiltinDetumbling);
    }

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestExperimentalDetumbling)
    {
        EXPECT_CALL(_coordinator, EnableExperimentalDetumbling());

        ExpectFrame(12, 0);

        Run(12, adcs::AdcsMode::ExperimentalDetumbling);
    }

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestSunpointing)
    {
        EXPECT_CALL(_coordinator, EnableSunPointing());

        ExpectFrame(13, 0);

        Run(13, adcs::AdcsMode::ExperimentalSunpointing);
    }

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestStoppingAdcsFailure)
    {
        EXPECT_CALL(_coordinator, Stop()).WillOnce(Return(OSResult::IOError));

        ExpectFrame(10, 1);

        Run(10, adcs::AdcsMode::Stopped);
    }

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestDisablingAdcsFailure)
    {
        EXPECT_CALL(_coordinator, Disable()).WillOnce(Return(OSResult::IOError));

        ExpectFrame(11, 1);

        Run(11, adcs::AdcsMode::Disabled);
    }

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestBuiltinDetumblingFailure)
    {
        EXPECT_CALL(_coordinator, EnableBuiltinDetumbling()).WillOnce(Return(OSResult::IOError));

        ExpectFrame(11, 1);

        Run(11, adcs::AdcsMode::BuiltinDetumbling);
    }

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestExperimentalDetumblingFailure)
    {
        EXPECT_CALL(_coordinator, EnableExperimentalDetumbling()).WillOnce(Return(OSResult::IOError));

        ExpectFrame(12, 1);

        Run(12, adcs::AdcsMode::ExperimentalDetumbling);
    }

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestSunpointingFailure)
    {
        EXPECT_CALL(_coordinator, EnableSunPointing()).WillOnce(Return(OSResult::IOError));

        ExpectFrame(13, 1);

        Run(13, adcs::AdcsMode::ExperimentalSunpointing);
    }
}
