#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/comm.hpp"
#include "mock/experiment.hpp"

#include "obc/telecommands/experiments.hpp"

struct SetupSunSExperimentMock : experiments::suns::ISetupSunSExperiment
{
    MOCK_METHOD1(SetParameters, void(experiments::suns::SunSExperimentParams parameters));
    MOCK_METHOD1(SetOutputFiles, void(const char* baseName));
};

using telecommunication::downlink::DownlinkAPID;
using testing::ElementsAre;
using testing::_;
using testing::Return;
using testing::AllOf;
using testing::Property;
using testing::Eq;
using testing::StrEq;
using namespace std::chrono_literals;

using Params = experiments::suns::SunSExperimentParams;

namespace
{
    class PerformSunSExperimentTelecommandTest : public testing::Test
    {
      protected:
        template <typename... T> void Run(T... params);

        testing::NiceMock<ExperimentControllerMock> _controller;
        testing::NiceMock<SetupSunSExperimentMock> _suns;
        obc::telecommands::PerformSunSExperiment _telecommand{_controller, _suns};

        testing::NiceMock<TransmitterMock> _transmitter;
    };

    template <typename... T> void PerformSunSExperimentTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(_transmitter, buffer);
    }

    TEST_F(PerformSunSExperimentTelecommandTest, RespondWithErrorOnTooShortFrame)
    {
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(43, 1))));
        Run(43, 1, 2, 3, 4);
    }

    TEST_F(PerformSunSExperimentTelecommandTest, RespondWithErrorOnNoFileName)
    {
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(43, 1))));
        Run(43, 1, 2, 3, 4, 5, 6, 0);
    }

    TEST_F(PerformSunSExperimentTelecommandTest, RespondWithErrorWhenControllerBusy)
    {
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(43, 2))));

        ON_CALL(this->_controller, RequestExperiment(_)).WillByDefault(Return(false));

        Run(43, 1, 2, 3, 4, 5, 6, '/', 'e', 'x', 'p', 0);
    }

    TEST_F(PerformSunSExperimentTelecommandTest, ShouldSetExperimentParameters)
    {
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(43, 0))));

        EXPECT_CALL(this->_suns,
            SetParameters(AllOf(                                 //
                Property(&Params::Gain, Eq(1)),                  //
                Property(&Params::ITime, Eq(2)),                 //
                Property(&Params::SamplesCount, Eq(3)),          //
                Property(&Params::ShortDelay, Eq(4s)),           //
                Property(&Params::SamplingSessionsCount, Eq(5)), //
                Property(&Params::LongDelay, Eq(6min))           //
                )));

        EXPECT_CALL(this->_suns, SetOutputFiles(StrEq("/exp")));

        EXPECT_CALL(this->_controller, RequestExperiment(4)).WillOnce(Return(true));

        Run(43, 1, 2, 3, 4, 5, 6, '/', 'e', 'x', 'p', 0);
    }
}
