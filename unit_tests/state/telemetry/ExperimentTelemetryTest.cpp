#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "experiments/experiments.h"
#include "telemetry/Experiments.hpp"

namespace
{
    using testing::Eq;

    TEST(ExperimentTelemetryTest, TestDefaultConstruction)
    {
        telemetry::ExperimentTelemetry object;
        ASSERT_THAT(object.CurrentExperiment(), Eq(0));
        ASSERT_THAT(object.StartResult(), Eq(experiments::StartResult::Success));
        ASSERT_THAT(object.IterationResult(), Eq(experiments::IterationResult::None));
    }

    TEST(ExperimentTelemetryTest, TestCustomConstruction)
    {
        telemetry::ExperimentTelemetry object(12, experiments::StartResult::Failure, experiments::IterationResult::LoopImmediately);
        ASSERT_THAT(object.CurrentExperiment(), Eq(12));
        ASSERT_THAT(object.StartResult(), Eq(experiments::StartResult::Failure));
        ASSERT_THAT(object.IterationResult(), Eq(experiments::IterationResult::LoopImmediately));
    }

    TEST(ExperimentTelemetryTest, TestCustomConstructionFromEmptyExperiment)
    {
        experiments::ExperimentState state;
        telemetry::ExperimentTelemetry object(state);
        ASSERT_THAT(object.CurrentExperiment(), Eq(0));
        ASSERT_THAT(object.StartResult(), Eq(experiments::StartResult::Success));
        ASSERT_THAT(object.IterationResult(), Eq(experiments::IterationResult::None));
    }

    TEST(ExperimentTelemetryTest, TestCustomConstructionFromFullExperiment)
    {
        experiments::ExperimentState state;
        state.CurrentExperiment = Option<experiments::ExperimentCode>::Some(12);
        state.LastStartResult = Option<experiments::StartResult>::Some(experiments::StartResult::Failure);
        state.LastIterationResult = Option<experiments::IterationResult>::Some(experiments::IterationResult::LoopImmediately);
        state.IterationCounter = 213456;
        telemetry::ExperimentTelemetry object(state);
        ASSERT_THAT(object.CurrentExperiment(), Eq(12));
        ASSERT_THAT(object.StartResult(), Eq(experiments::StartResult::Failure));
        ASSERT_THAT(object.IterationResult(), Eq(experiments::IterationResult::LoopImmediately));
    }

    TEST(ExperimentTelemetryTest, TestSerialization)
    {
        std::uint8_t expected[] = {12, num(experiments::StartResult::Failure), num(experiments::IterationResult::LoopImmediately)};
        std::array<std::uint8_t, (telemetry::ExperimentTelemetry::BitSize() + 7) / 8> buffer;
        telemetry::ExperimentTelemetry object(12, experiments::StartResult::Failure, experiments::IterationResult::LoopImmediately);
        BitWriter writer(buffer);
        object.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(telemetry::ExperimentTelemetry::BitSize()));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }
}
