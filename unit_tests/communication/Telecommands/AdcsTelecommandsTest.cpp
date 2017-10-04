#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "mock/AdcsMocks.hpp"
#include "mock/HasStateMock.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/adcs.hpp"
#include "state/struct.h"

using telecommunication::downlink::DownlinkAPID;
using testing::ElementsAre;
using testing::Return;
using testing::ReturnRef;
using testing::_;
using testing::Eq;

namespace
{
    class SetAdcsModeExperimentTelecommandTest : public testing::Test
    {
      protected:
        template <typename... T> void Run(T... params);

        void ExpectFrame(std::uint8_t correlationId, std::uint8_t errorCode);

        testing::NiceMock<TransmitterMock> _transmitter;
        testing::StrictMock<AdcsCoordinatorMock> _coordinator;

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

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestIncomplete)
    {
        ExpectFrame(13, 1);

        Run(13);
    }

    TEST_F(SetAdcsModeExperimentTelecommandTest, TestEmpty)
    {
        ExpectFrame(0, 1);

        Run();
    }

    class SetBuiltinDetumblingBlockMaskTelecommandTest : public testing::Test
    {
      protected:
        SetBuiltinDetumblingBlockMaskTelecommandTest();

        template <typename... T> void Run(T... params);

        void ExpectFrame(std::uint8_t correlationId, std::uint8_t errorCode);

        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<AdcsCoordinatorMock> _coordinator;
        testing::NiceMock<OSMock> _os;
        SystemState _stateObject;
        HasStateMock<SystemState> _hasState;
        OSReset _reset;
        obc::telecommands::SetBuiltinDetumblingBlockMaskTelecommand _telecommand;
    };

    SetBuiltinDetumblingBlockMaskTelecommandTest::SetBuiltinDetumblingBlockMaskTelecommandTest() : _telecommand{_hasState, _coordinator}
    {
        ON_CALL(_hasState, MockGetState()).WillByDefault(ReturnRef(_stateObject));
        _reset = InstallProxy(&_os);
    }

    template <typename... T> void SetBuiltinDetumblingBlockMaskTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(_transmitter, buffer);
    }

    void SetBuiltinDetumblingBlockMaskTelecommandTest::ExpectFrame(std::uint8_t correlationId, std::uint8_t errorCode)
    {
        EXPECT_CALL(
            _transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::SetInternalDetumblingMode, 0, ElementsAre(correlationId, errorCode))));
    }

    TEST_F(SetBuiltinDetumblingBlockMaskTelecommandTest, TestEmpty)
    {
        ExpectFrame(0, 1);

        Run(0);
    }

    TEST_F(SetBuiltinDetumblingBlockMaskTelecommandTest, TestIncomplete)
    {
        ExpectFrame(10, 1);

        Run(10);
    }

    TEST_F(SetBuiltinDetumblingBlockMaskTelecommandTest, TestPersistentSateFail)
    {
        EXPECT_CALL(_os, TakeSemaphore(_, _)).WillOnce(Return(OSResult::IOError));
        ExpectFrame(10, 2);

        Run(10, 0);
    }

    TEST_F(SetBuiltinDetumblingBlockMaskTelecommandTest, TestSuccess)
    {
        EXPECT_CALL(_coordinator, SetBlockMode(adcs::AdcsMode::BuiltinDetumbling, true));

        ExpectFrame(11, 0);

        Run(11, 12);

        state::AdcsState adcs;
        _stateObject.PersistentState.Get(adcs);
        ASSERT_THAT(adcs.IsInternalDetumblingDisabled(), Eq(true));
    }
}
