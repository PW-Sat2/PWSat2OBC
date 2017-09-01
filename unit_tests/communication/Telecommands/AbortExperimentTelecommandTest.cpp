#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/comm.hpp"
#include "mock/experiment.hpp"
#include "obc/telecommands/experiments.hpp"

using telecommunication::downlink::DownlinkAPID;
using testing::ElementsAre;

namespace
{
    class AbortExperimentTelecommandTest : public testing::Test
    {
      protected:
        template <typename... T> void Run(T... params);

        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<ExperimentControllerMock> _experiments;

        obc::telecommands::AbortExperiment _telecommand{_experiments};
    };

    template <typename... T> void AbortExperimentTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(_transmitter, buffer);
    }

    TEST_F(AbortExperimentTelecommandTest, ShouldAbortExperiment)
    {
        EXPECT_CALL(_experiments, AbortExperiment());

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Experiment, 0, ElementsAre(0x24, 0))));

        Run(0x24);
    }

    TEST_F(AbortExperimentTelecommandTest, ShouldRespondWithErrorOnEmptyFrame)
    {
        EXPECT_CALL(_experiments, AbortExperiment()).Times(0);

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Experiment, 0, ElementsAre(0, 1))));

        Run();
    }
}
