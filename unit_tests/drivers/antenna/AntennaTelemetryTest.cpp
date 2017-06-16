#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "antenna/telemetry.hpp"
#include "base/BitWriter.hpp"

using testing::Eq;
using namespace devices::antenna;
using namespace std::chrono_literals;

namespace
{
    TEST(ActivationCounts, DefaultConstruction)
    {
        ActivationCounts counts;
        ASSERT_THAT(counts.GetActivationCount(ANTENNA1_ID), Eq(0));
        ASSERT_THAT(counts.GetActivationCount(ANTENNA2_ID), Eq(0));
        ASSERT_THAT(counts.GetActivationCount(ANTENNA3_ID), Eq(0));
        ASSERT_THAT(counts.GetActivationCount(ANTENNA4_ID), Eq(0));
    }

    TEST(ActivationCounts, CustomConstruction)
    {
        ActivationCounts counts(0xff, 0xf0, 0x0f, 0x55);
        ASSERT_THAT(counts.GetActivationCount(ANTENNA1_ID), Eq(0xff));
        ASSERT_THAT(counts.GetActivationCount(ANTENNA2_ID), Eq(0xf0));
        ASSERT_THAT(counts.GetActivationCount(ANTENNA3_ID), Eq(0x0f));
        ASSERT_THAT(counts.GetActivationCount(ANTENNA4_ID), Eq(0x55));
    }

    TEST(ActivationCounts, CustomState)
    {
        ActivationCounts counts;
        counts.SetActivationCount(ANTENNA1_ID, 0xff);
        counts.SetActivationCount(ANTENNA2_ID, 0xf0);
        counts.SetActivationCount(ANTENNA3_ID, 0x0f);
        counts.SetActivationCount(ANTENNA4_ID, 0x55);
        ASSERT_THAT(counts.GetActivationCount(ANTENNA1_ID), Eq(0xff));
        ASSERT_THAT(counts.GetActivationCount(ANTENNA2_ID), Eq(0xf0));
        ASSERT_THAT(counts.GetActivationCount(ANTENNA3_ID), Eq(0x0f));
        ASSERT_THAT(counts.GetActivationCount(ANTENNA4_ID), Eq(0x55));
    }

    TEST(ActivationCounts, Serialization)
    {
        std::uint8_t buffer[4];
        std::uint8_t expected[] = {0xff, 0xf0, 0x0f, 0x55};
        ActivationCounts counts;
        counts.SetActivationCount(ANTENNA1_ID, 0xff);
        counts.SetActivationCount(ANTENNA2_ID, 0xf0);
        counts.SetActivationCount(ANTENNA3_ID, 0x0f);
        counts.SetActivationCount(ANTENNA4_ID, 0x55);
        BitWriter writer(buffer);
        counts.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(32u));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }

    TEST(ActivationTimes, DefaultConstruction)
    {
        ActivationTimes times;
        ASSERT_THAT(times.GetActivationTime(ANTENNA1_ID), Eq(0s));
        ASSERT_THAT(times.GetActivationTime(ANTENNA2_ID), Eq(0s));
        ASSERT_THAT(times.GetActivationTime(ANTENNA3_ID), Eq(0s));
        ASSERT_THAT(times.GetActivationTime(ANTENNA4_ID), Eq(0s));
    }

    TEST(ActivationTimes, CustomConstruction)
    {
        ActivationTimes times(1s, 128s, 4095s, 1023s);
        ASSERT_THAT(times.GetActivationTime(ANTENNA1_ID), Eq(1s));
        ASSERT_THAT(times.GetActivationTime(ANTENNA2_ID), Eq(128s));
        ASSERT_THAT(times.GetActivationTime(ANTENNA3_ID), Eq(4095s));
        ASSERT_THAT(times.GetActivationTime(ANTENNA4_ID), Eq(1023s));
    }

    TEST(ActivationTimes, CustomState)
    {
        ActivationTimes times;
        times.SetActivationTime(ANTENNA1_ID, 1s);
        times.SetActivationTime(ANTENNA2_ID, 128s);
        times.SetActivationTime(ANTENNA3_ID, 4095s);
        times.SetActivationTime(ANTENNA4_ID, 1023s);
        ASSERT_THAT(times.GetActivationTime(ANTENNA1_ID), Eq(1s));
        ASSERT_THAT(times.GetActivationTime(ANTENNA2_ID), Eq(128s));
        ASSERT_THAT(times.GetActivationTime(ANTENNA3_ID), Eq(4095s));
        ASSERT_THAT(times.GetActivationTime(ANTENNA4_ID), Eq(1023s));
    }

    TEST(ActivationTimes, Serialization)
    {
        std::uint8_t buffer[6];
        std::uint8_t expected[] = {0x12, 0xc0, 0xab, 0xed, 0xaf, 0xcb};
        ActivationTimes times;
        times.SetActivationTime(ANTENNA1_ID, 0x012s);
        times.SetActivationTime(ANTENNA2_ID, 0xabcs);
        times.SetActivationTime(ANTENNA3_ID, 0xfeds);
        times.SetActivationTime(ANTENNA4_ID, 0xcbas);

        BitWriter writer(buffer);
        times.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(48u));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }

    TEST(AntennaTelemetry, DefaultConstruction)
    {
        AntennaTelemetry telemetry;
        auto& counts1 = telemetry.GetActivationCounts(ANTENNA_PRIMARY_CHANNEL);
        auto& counts2 = telemetry.GetActivationCounts(ANTENNA_BACKUP_CHANNEL);
        auto& times1 = telemetry.GetActivationTimes(ANTENNA_PRIMARY_CHANNEL);
        auto& times2 = telemetry.GetActivationTimes(ANTENNA_BACKUP_CHANNEL);
        ASSERT_THAT(telemetry.GetDeploymentStatus(), Eq(0));
        ASSERT_THAT(counts1.GetActivationCount(ANTENNA1_ID), Eq(0));
        ASSERT_THAT(counts1.GetActivationCount(ANTENNA2_ID), Eq(0));
        ASSERT_THAT(counts1.GetActivationCount(ANTENNA3_ID), Eq(0));
        ASSERT_THAT(counts1.GetActivationCount(ANTENNA4_ID), Eq(0));
        ASSERT_THAT(counts2.GetActivationCount(ANTENNA1_ID), Eq(0));
        ASSERT_THAT(counts2.GetActivationCount(ANTENNA2_ID), Eq(0));
        ASSERT_THAT(counts2.GetActivationCount(ANTENNA3_ID), Eq(0));
        ASSERT_THAT(counts2.GetActivationCount(ANTENNA4_ID), Eq(0));

        ASSERT_THAT(times1.GetActivationTime(ANTENNA1_ID), Eq(0s));
        ASSERT_THAT(times1.GetActivationTime(ANTENNA2_ID), Eq(0s));
        ASSERT_THAT(times1.GetActivationTime(ANTENNA3_ID), Eq(0s));
        ASSERT_THAT(times1.GetActivationTime(ANTENNA4_ID), Eq(0s));
        ASSERT_THAT(times2.GetActivationTime(ANTENNA1_ID), Eq(0s));
        ASSERT_THAT(times2.GetActivationTime(ANTENNA2_ID), Eq(0s));
        ASSERT_THAT(times2.GetActivationTime(ANTENNA3_ID), Eq(0s));
        ASSERT_THAT(times2.GetActivationTime(ANTENNA4_ID), Eq(0s));
    }

    TEST(AntennaTelemetry, CustomState)
    {
        AntennaTelemetry telemetry;

        telemetry.SetActivationCounts(ANTENNA_PRIMARY_CHANNEL, ActivationCounts(1, 2, 3, 4));
        telemetry.SetActivationCounts(ANTENNA_BACKUP_CHANNEL, ActivationCounts(5, 6, 7, 8));

        telemetry.SetActivationTimes(ANTENNA_PRIMARY_CHANNEL, ActivationTimes(10s, 20s, 30s, 40s));
        telemetry.SetActivationTimes(ANTENNA_BACKUP_CHANNEL, ActivationTimes(50s, 60s, 70s, 80s));
        telemetry.SetDeploymentStatus(0xa5);

        auto& counts1 = telemetry.GetActivationCounts(ANTENNA_PRIMARY_CHANNEL);
        auto& counts2 = telemetry.GetActivationCounts(ANTENNA_BACKUP_CHANNEL);
        auto& times1 = telemetry.GetActivationTimes(ANTENNA_PRIMARY_CHANNEL);
        auto& times2 = telemetry.GetActivationTimes(ANTENNA_BACKUP_CHANNEL);

        ASSERT_THAT(telemetry.GetDeploymentStatus(), Eq(0xa5));
        ASSERT_THAT(counts1.GetActivationCount(ANTENNA1_ID), Eq(1));
        ASSERT_THAT(counts1.GetActivationCount(ANTENNA2_ID), Eq(2));
        ASSERT_THAT(counts1.GetActivationCount(ANTENNA3_ID), Eq(3));
        ASSERT_THAT(counts1.GetActivationCount(ANTENNA4_ID), Eq(4));
        ASSERT_THAT(counts2.GetActivationCount(ANTENNA1_ID), Eq(5));
        ASSERT_THAT(counts2.GetActivationCount(ANTENNA2_ID), Eq(6));
        ASSERT_THAT(counts2.GetActivationCount(ANTENNA3_ID), Eq(7));
        ASSERT_THAT(counts2.GetActivationCount(ANTENNA4_ID), Eq(8));

        ASSERT_THAT(times1.GetActivationTime(ANTENNA1_ID), Eq(10s));
        ASSERT_THAT(times1.GetActivationTime(ANTENNA2_ID), Eq(20s));
        ASSERT_THAT(times1.GetActivationTime(ANTENNA3_ID), Eq(30s));
        ASSERT_THAT(times1.GetActivationTime(ANTENNA4_ID), Eq(40s));
        ASSERT_THAT(times2.GetActivationTime(ANTENNA1_ID), Eq(50s));
        ASSERT_THAT(times2.GetActivationTime(ANTENNA2_ID), Eq(60s));
        ASSERT_THAT(times2.GetActivationTime(ANTENNA3_ID), Eq(70s));
        ASSERT_THAT(times2.GetActivationTime(ANTENNA4_ID), Eq(80s));
    }

    TEST(AntennaTelemetry, Serialization)
    {
        std::uint8_t buffer[21];
        std::uint8_t expected[] = {
            0x5a, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x0a, 0x40, 0x01, 0x1e, 0x80, 0x02, 0x32, 0xc0, 0x03, 0x46, 0x00, 0x05};
        AntennaTelemetry telemetry;

        telemetry.SetActivationCounts(ANTENNA_PRIMARY_CHANNEL, ActivationCounts(1, 2, 3, 4));
        telemetry.SetActivationCounts(ANTENNA_BACKUP_CHANNEL, ActivationCounts(5, 6, 7, 8));

        telemetry.SetActivationTimes(ANTENNA_PRIMARY_CHANNEL, ActivationTimes(10s, 20s, 30s, 40s));
        telemetry.SetActivationTimes(ANTENNA_BACKUP_CHANNEL, ActivationTimes(50s, 60s, 70s, 80s));
        telemetry.SetDeploymentStatus(0x5a);

        BitWriter writer(buffer);
        telemetry.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(168u));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }

    struct AntennaTelemetryTest : public testing::TestWithParam<std::tuple<AntennaChannel, AntennaId, uint8_t>>
    {
    };

    TEST_P(AntennaTelemetryTest, DeploymentStatus)
    {
        const auto channel = std::get<0>(GetParam());
        const auto antenna = std::get<1>(GetParam());
        const auto result = std::get<2>(GetParam());

        AntennaTelemetry telemetry;
        telemetry.SetDeploymentStatus(channel, antenna, true);
        ASSERT_THAT(telemetry.GetDeploymentStatus(), Eq(result));
    }

    INSTANTIATE_TEST_CASE_P(AntennaTelemetryDeploymentStatus,
        AntennaTelemetryTest,
        testing::Values(std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 0x01),
            std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 0x02),
            std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 0x04),
            std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 0x08),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 0x10),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 0x20),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 0x40),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 0x80)), );
}
