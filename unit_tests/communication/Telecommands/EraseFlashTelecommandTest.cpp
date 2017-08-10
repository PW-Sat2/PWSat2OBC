#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/comm.hpp"
#include "mock/experiment.hpp"
#include "obc/telecommands/flash.hpp"

using testing::Return;
using testing::ElementsAre;
using testing::_;
using telecommunication::downlink::DownlinkAPID;
using experiment::erase_flash::Status;

namespace
{
    class EraseFlashTelecommandTest : public testing::Test
    {
      protected:
        template <typename... T> void Run(T... params);

        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<ExperimentControllerMock> _experiments;

        obc::telecommands::EraseFlashTelecommand _telecommand{_experiments};
    };

    template <typename... T> void EraseFlashTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(_transmitter, buffer);
    }

    TEST_F(EraseFlashTelecommandTest, ShouldStartEraseFlashExperiment)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x67, 0, num(Status::Requested)))));
        EXPECT_CALL(_experiments, RequestExperiment(7)).WillOnce(Return(true));
        Run(0x67);
    }

    TEST_F(EraseFlashTelecommandTest, ShouldRespondWithErrorOnEmptyFrame)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0, 1))));
        EXPECT_CALL(_experiments, RequestExperiment(_)).Times(0);
        Run();
    }

    TEST_F(EraseFlashTelecommandTest, ShouldRespondWithErrorOnWhenRequestDenied)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x67, 2))));
        EXPECT_CALL(_experiments, RequestExperiment(_)).WillOnce(Return(false));
        Run(0x67);
    }
}
