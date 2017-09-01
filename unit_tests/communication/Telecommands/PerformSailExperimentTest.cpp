#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "mock/comm.hpp"
#include "mock/experiment.hpp"
#include "obc/telecommands/experiments.hpp"

namespace
{
    using obc::telecommands::PerformSailExperiment;
    using telecommunication::downlink::DownlinkFrame;
    using telecommunication::downlink::DownlinkAPID;

    using testing::_;
    using testing::Eq;
    using testing::Invoke;
    using testing::Return;

    class PerformSailExperimentTest : public testing::Test
    {
      protected:
        PerformSailExperimentTest();

        TransmitterMock transmitter;
        ExperimentControllerMock controller;
        PerformSailExperiment command;
    };

    PerformSailExperimentTest::PerformSailExperimentTest() : command(controller)
    {
    }

    TEST_F(PerformSailExperimentTest, TestEmptyParameterList)
    {
        EXPECT_CALL(controller, RequestExperiment(_)).Times(0);
        EXPECT_CALL(transmitter, SendFrame(_)).WillOnce(Invoke([](gsl::span<const std::uint8_t> frame) {
            EXPECT_THAT(frame.size_bytes(), Eq(5));
            EXPECT_THAT(frame[0], Eq(num(DownlinkAPID::Experiment)));
            EXPECT_THAT(frame[1], Eq(0));
            EXPECT_THAT(frame[2], Eq(0));
            EXPECT_THAT(frame[3], Eq(0));
            EXPECT_THAT(frame[4], Eq(1));
            return true;
        }));

        command.Handle(transmitter, {});
    }

    TEST_F(PerformSailExperimentTest, TestExperimentRefused)
    {
        EXPECT_CALL(controller, RequestExperiment(7)).WillOnce(Return(false));
        EXPECT_CALL(transmitter, SendFrame(_)).WillOnce(Invoke([](gsl::span<const std::uint8_t> frame) {
            EXPECT_THAT(frame.size_bytes(), Eq(5));
            EXPECT_THAT(frame[0], Eq(num(DownlinkAPID::Experiment)));
            EXPECT_THAT(frame[1], Eq(0));
            EXPECT_THAT(frame[2], Eq(0));
            EXPECT_THAT(frame[3], Eq(1));
            EXPECT_THAT(frame[4], Eq(2));
            return true;
        }));

        const std::uint8_t buffer[] = {0x1};
        command.Handle(transmitter, gsl::make_span(buffer));
    }

    TEST_F(PerformSailExperimentTest, TestExperimentStarted)
    {
        EXPECT_CALL(controller, RequestExperiment(7)).WillOnce(Return(true));
        EXPECT_CALL(transmitter, SendFrame(_)).WillOnce(Invoke([](gsl::span<const std::uint8_t> frame) {
            EXPECT_THAT(frame.size_bytes(), Eq(5));
            EXPECT_THAT(frame[0], Eq(num(DownlinkAPID::Experiment)));
            EXPECT_THAT(frame[1], Eq(0));
            EXPECT_THAT(frame[2], Eq(0));
            EXPECT_THAT(frame[3], Eq(1));
            EXPECT_THAT(frame[4], Eq(0));
            return true;
        }));

        const std::uint8_t buffer[] = {0x1};
        command.Handle(transmitter, gsl::make_span(buffer));
    }
}
