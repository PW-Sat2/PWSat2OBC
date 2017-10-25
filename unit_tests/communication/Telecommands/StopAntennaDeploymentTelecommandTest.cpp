#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "mock/HasStateMock.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/antenna.hpp"
#include "os/os.hpp"

using telecommunication::downlink::DownlinkAPID;
using testing::ElementsAre;
using testing::Eq;
using testing::Return;
using testing::ReturnRef;
using testing::_;

namespace
{
    class StopAntennaDeploymentTelecommandTest : public testing::Test
    {
      protected:
        StopAntennaDeploymentTelecommandTest();

        template <typename... T> void Run(T... params);

        void ExpectFrame(std::uint8_t correlationId, std::uint8_t errorCode);

        void CheckDeploymentState(bool disabled);

        testing::NiceMock<HasStateMock<SystemState>> stateContainer;
        testing::NiceMock<TransmitterMock> transmitter;

        testing::NiceMock<OSMock> os;
        OSReset guard;

        SystemState _state;
        obc::telecommands::SetAntennaDeploymentMaskTelecommand _telecommand;
    };

    StopAntennaDeploymentTelecommandTest::StopAntennaDeploymentTelecommandTest() : _telecommand(stateContainer)
    {
        this->guard = InstallProxy(&os);
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
        ON_CALL(stateContainer, MockGetState()).WillByDefault(ReturnRef(_state));
    }

    template <typename... T> void StopAntennaDeploymentTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(transmitter, buffer);
    }

    void StopAntennaDeploymentTelecommandTest::ExpectFrame(std::uint8_t correlationId, std::uint8_t errorCode)
    {
        EXPECT_CALL(
            transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::DisableAntennaDeployment, 0, ElementsAre(correlationId, errorCode))));
    }

    void StopAntennaDeploymentTelecommandTest::CheckDeploymentState(bool disabled)
    {
        state::AntennaConfiguration configuration;
        auto& persistentState = _state.PersistentState;
        persistentState.Get(configuration);
        ASSERT_THAT(configuration.IsDeploymentDisabled(), Eq(disabled));
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
        Run(0x22, 1);
        CheckDeploymentState(true);
    }

    TEST_F(StopAntennaDeploymentTelecommandTest, TestEnablingDeploymentFrame)
    {
        ExpectFrame(0x22, 0);
        Run(0x22, 0);
        CheckDeploymentState(false);
    }

    TEST_F(StopAntennaDeploymentTelecommandTest, TestUpdateFailure)
    {
        EXPECT_CALL(os, TakeSemaphore(_, _)).WillOnce(Return(OSResult::IOError)).WillRepeatedly(Return(OSResult::Success));
        ExpectFrame(0x22, -2);
        Run(0x22, 1);
        CheckDeploymentState(false);
    }
}
