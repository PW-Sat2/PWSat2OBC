#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/comm.hpp"
#include "mock/experiment.hpp"
#include "obc/telecommands/experiments.hpp"

using telecommunication::downlink::DownlinkAPID;
using testing::ElementsAre;
using testing::_;
using testing::Return;
using namespace std::chrono_literals;

struct SetupDetumblingExperimentMock : public experiment::adcs::ISetupDetumblingExperiment
{
    MOCK_METHOD1(Duration, void(std::chrono::seconds duration));
    MOCK_METHOD1(SampleRate, void(std::chrono::seconds interval));
};

namespace
{
    class PerformDetumblingExperimentTelecommandTest : public testing::Test
    {
      protected:
        template <typename... T> void Run(T... params);

        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<ExperimentControllerMock> _experiments;
        testing::NiceMock<SetupDetumblingExperimentMock> _setup;

        obc::telecommands::PerformDetumblingExperiment _telecommand{_experiments, _setup};
    };

    template <typename... T> void PerformDetumblingExperimentTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(_transmitter, buffer);
    }

    TEST_F(PerformDetumblingExperimentTelecommandTest, ShouldRespondWithErrorOnEmptyFrame)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0, 1))));
        EXPECT_CALL(this->_setup, Duration(_)).Times(0);
        EXPECT_CALL(this->_experiments, RequestExperiment(_)).Times(0);

        Run();
    }

    TEST_F(PerformDetumblingExperimentTelecommandTest, ShouldRespondWithErrorOnTooShortFrame)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0xDE, 1))));
        EXPECT_CALL(this->_setup, Duration(_)).Times(0);
        EXPECT_CALL(this->_experiments, RequestExperiment(_)).Times(0);

        Run(0xDE, 0xAD, 0xBE, 0xEF);
    }

    TEST_F(PerformDetumblingExperimentTelecommandTest, ShouldSetDetumblingParametersAndRequestExperiment)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x94, 0))));
        EXPECT_CALL(this->_setup, Duration(0x04030201s));
        EXPECT_CALL(this->_setup, SampleRate(10s));
        EXPECT_CALL(this->_experiments, RequestExperiment(experiment::adcs::DetumblingExperiment::Code)).WillOnce(Return(true));

        Run(0x94, 0x01, 0x02, 0x03, 0x04, 0x0A);
    }

    TEST_F(PerformDetumblingExperimentTelecommandTest, ShouldRespondWithErrorIfRequestFails)
    {
        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x94, 2))));
        EXPECT_CALL(this->_experiments, RequestExperiment(experiment::adcs::DetumblingExperiment::Code)).WillOnce(Return(false));

        Run(0x94, 0x01, 0x02, 0x03, 0x04, 0x0A);
    }
}
