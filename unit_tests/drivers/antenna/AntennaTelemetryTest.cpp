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

    TEST(ActivationCounts, SerializationInRange)
    {
        std::uint8_t buffer[2] = {0};
        std::uint8_t expected[] = {0b01011010, 0b00001111};
        ActivationCounts counts;
        counts.SetActivationCount(ANTENNA1_ID, 2);
        counts.SetActivationCount(ANTENNA2_ID, 3);
        counts.SetActivationCount(ANTENNA3_ID, 5);
        counts.SetActivationCount(ANTENNA4_ID, 7);
        BitWriter writer(buffer);
        counts.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(12u));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }

    TEST(ActivationCounts, SerializationOverflow)
    {
        std::uint8_t buffer[2] = {0};
        std::uint8_t expected[] = {0xff, 0x0f};
        ActivationCounts counts;
        counts.SetActivationCount(ANTENNA1_ID, 0xff);
        counts.SetActivationCount(ANTENNA2_ID, 0xf0);
        counts.SetActivationCount(ANTENNA3_ID, 0x0f);
        counts.SetActivationCount(ANTENNA4_ID, 0x55);
        BitWriter writer(buffer);
        counts.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(12u));
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

    TEST(ActivationTimes, SerializationInRange)
    {
        std::uint8_t buffer[4];
        std::uint8_t expected[] = {0b11111111, 0b01111111, 0b00111111, 0b00100000};
        ActivationTimes times;
        times.SetActivationTime(ANTENNA1_ID, 511s);
        times.SetActivationTime(ANTENNA2_ID, 255s);
        times.SetActivationTime(ANTENNA3_ID, 127s);
        times.SetActivationTime(ANTENNA4_ID, 64s);

        BitWriter writer(buffer);
        times.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(32u));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }

    TEST(ActivationTimes, SerializationOverflow)
    {
        std::uint8_t buffer[4];
        std::uint8_t expected[] = {43, 0xFF, 0xFF, 0xFF};
        ActivationTimes times;
        times.SetActivationTime(ANTENNA1_ID, 87s);
        times.SetActivationTime(ANTENNA2_ID, 512s);
        times.SetActivationTime(ANTENNA3_ID, 678s);
        times.SetActivationTime(ANTENNA4_ID, 897s);

        BitWriter writer(buffer);
        times.Write(writer);
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(32u));
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
    }

    TEST(AntennaTelemetry, DefaultConstruction)
    {
        AntennaTelemetry telemetry;
        auto& counts1 = telemetry.GetActivationCounts(ANTENNA_PRIMARY_CHANNEL);
        auto& counts2 = telemetry.GetActivationCounts(ANTENNA_BACKUP_CHANNEL);
        auto& times1 = telemetry.GetActivationTimes(ANTENNA_PRIMARY_CHANNEL);
        auto& times2 = telemetry.GetActivationTimes(ANTENNA_BACKUP_CHANNEL);
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

        ASSERT_THAT(telemetry.GetChannelStatus(ANTENNA_PRIMARY_CHANNEL), Eq(ChannelStatus::None));
        ASSERT_THAT(telemetry.GetChannelStatus(ANTENNA_BACKUP_CHANNEL), Eq(ChannelStatus::None));
    }

    TEST(AntennaTelemetry, CustomState)
    {
        AntennaTelemetry telemetry;

        telemetry.SetActivationCounts(ANTENNA_PRIMARY_CHANNEL, ActivationCounts(1, 2, 3, 4));
        telemetry.SetActivationCounts(ANTENNA_BACKUP_CHANNEL, ActivationCounts(5, 6, 7, 8));

        telemetry.SetActivationTimes(ANTENNA_PRIMARY_CHANNEL, ActivationTimes(10s, 20s, 30s, 40s));
        telemetry.SetActivationTimes(ANTENNA_BACKUP_CHANNEL, ActivationTimes(50s, 60s, 70s, 80s));

        telemetry.SetChannelStatus(
            ANTENNA_PRIMARY_CHANNEL, ChannelStatus::Armed | ChannelStatus::IndependentBurn | ChannelStatus::IgnoringSwitches);
        telemetry.SetChannelStatus(ANTENNA_BACKUP_CHANNEL, ChannelStatus::IndependentBurn);

        telemetry.SetBurningStatus(ANTENNA_PRIMARY_CHANNEL, false, true, true, false);
        telemetry.SetBurningStatus(ANTENNA_BACKUP_CHANNEL, true, false, false, true);

        telemetry.SetTimeReached(ANTENNA_PRIMARY_CHANNEL, true, false, true, false);
        telemetry.SetTimeReached(ANTENNA_BACKUP_CHANNEL, false, true, false, true);

        telemetry.SetDeployedStatus(ANTENNA_PRIMARY_CHANNEL, false, true, false, true);
        telemetry.SetDeployedStatus(ANTENNA_BACKUP_CHANNEL, true, false, true, false);

        auto& counts1 = telemetry.GetActivationCounts(ANTENNA_PRIMARY_CHANNEL);
        auto& counts2 = telemetry.GetActivationCounts(ANTENNA_BACKUP_CHANNEL);
        auto& times1 = telemetry.GetActivationTimes(ANTENNA_PRIMARY_CHANNEL);
        auto& times2 = telemetry.GetActivationTimes(ANTENNA_BACKUP_CHANNEL);

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

        ASSERT_THAT(telemetry.GetChannelStatus(ANTENNA_PRIMARY_CHANNEL),
            Eq(ChannelStatus::Armed | ChannelStatus::IndependentBurn | ChannelStatus::IgnoringSwitches));
        ASSERT_THAT(telemetry.GetChannelStatus(ANTENNA_BACKUP_CHANNEL), Eq(ChannelStatus::IndependentBurn));

        ASSERT_THAT(telemetry.GetBurningStatus(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA1_ID), Eq(false));
        ASSERT_THAT(telemetry.GetBurningStatus(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA2_ID), Eq(true));
        ASSERT_THAT(telemetry.GetBurningStatus(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA3_ID), Eq(true));
        ASSERT_THAT(telemetry.GetBurningStatus(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA4_ID), Eq(false));

        ASSERT_THAT(telemetry.GetBurningStatus(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA1_ID), Eq(true));
        ASSERT_THAT(telemetry.GetBurningStatus(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA2_ID), Eq(false));
        ASSERT_THAT(telemetry.GetBurningStatus(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA3_ID), Eq(false));
        ASSERT_THAT(telemetry.GetBurningStatus(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA4_ID), Eq(true));

        ASSERT_THAT(telemetry.GetDeployedStatus(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA1_ID), Eq(false));
        ASSERT_THAT(telemetry.GetDeployedStatus(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA2_ID), Eq(true));
        ASSERT_THAT(telemetry.GetDeployedStatus(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA3_ID), Eq(false));
        ASSERT_THAT(telemetry.GetDeployedStatus(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA4_ID), Eq(true));

        ASSERT_THAT(telemetry.GetDeployedStatus(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA1_ID), Eq(true));
        ASSERT_THAT(telemetry.GetDeployedStatus(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA2_ID), Eq(false));
        ASSERT_THAT(telemetry.GetDeployedStatus(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA3_ID), Eq(true));
        ASSERT_THAT(telemetry.GetDeployedStatus(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA4_ID), Eq(false));

        ASSERT_THAT(telemetry.GetTimeReached(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA1_ID), Eq(true));
        ASSERT_THAT(telemetry.GetTimeReached(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA2_ID), Eq(false));
        ASSERT_THAT(telemetry.GetTimeReached(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA3_ID), Eq(true));
        ASSERT_THAT(telemetry.GetTimeReached(ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA4_ID), Eq(false));

        ASSERT_THAT(telemetry.GetTimeReached(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA1_ID), Eq(false));
        ASSERT_THAT(telemetry.GetTimeReached(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA2_ID), Eq(true));
        ASSERT_THAT(telemetry.GetTimeReached(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA3_ID), Eq(false));
        ASSERT_THAT(telemetry.GetTimeReached(ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA4_ID), Eq(true));
    }

    TEST(AntennaTelemetry, Serialization)
    {
        std::uint8_t buffer[20];
        std::uint8_t expected[] = {0x5A, 0xA5, 0x96, 0x57, 0x34, 0xD6, 0x7F, 0x81, 0xC2, 0x03, 0x45, 0x86, 0xC7, 0x08, 0x0A};
        AntennaTelemetry telemetry;

        telemetry.SetActivationCounts(ANTENNA_PRIMARY_CHANNEL, ActivationCounts(1, 2, 3, 4));
        telemetry.SetActivationCounts(ANTENNA_BACKUP_CHANNEL, ActivationCounts(5, 6, 7, 8));

        telemetry.SetActivationTimes(ANTENNA_PRIMARY_CHANNEL, ActivationTimes(10s, 20s, 30s, 40s));
        telemetry.SetActivationTimes(ANTENNA_BACKUP_CHANNEL, ActivationTimes(50s, 60s, 70s, 80s));

        telemetry.SetChannelStatus(
            ANTENNA_PRIMARY_CHANNEL, ChannelStatus::Armed | ChannelStatus::IndependentBurn | ChannelStatus::IgnoringSwitches);
        telemetry.SetChannelStatus(ANTENNA_BACKUP_CHANNEL, ChannelStatus::IndependentBurn);

        telemetry.SetBurningStatus(ANTENNA_PRIMARY_CHANNEL, false, true, true, false);
        telemetry.SetBurningStatus(ANTENNA_BACKUP_CHANNEL, true, false, false, true);

        telemetry.SetTimeReached(ANTENNA_PRIMARY_CHANNEL, true, false, true, false);
        telemetry.SetTimeReached(ANTENNA_BACKUP_CHANNEL, false, true, false, true);

        telemetry.SetDeployedStatus(ANTENNA_PRIMARY_CHANNEL, false, true, false, true);
        telemetry.SetDeployedStatus(ANTENNA_BACKUP_CHANNEL, true, false, true, false);

        BitWriter writer(buffer);
        telemetry.Write(writer);
        ASSERT_THAT(writer.Capture(), Eq(gsl::make_span(expected)));
        ASSERT_THAT(writer.Status(), Eq(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(118u));
    }

    struct AntennaTelemetryTest : public testing::TestWithParam<std::tuple<AntennaChannel, AntennaId, uint8_t, uint8_t>>
    {
    };

    //    TEST_P(AntennaTelemetryTest, GetDeploymentStatusPositive)
    //    {
    //        const auto channel = std::get<0>(GetParam());
    //        const auto antenna = std::get<1>(GetParam());
    //        const auto baseValue = std::get<2>(GetParam());
    //
    //        AntennaTelemetry telemetry;
    //    }
    //
    //    TEST_P(AntennaTelemetryTest, GetDeploymentStatusNegative)
    //    {
    //        const auto channel = std::get<0>(GetParam());
    //        const auto antenna = std::get<1>(GetParam());
    //        const auto baseValue = std::get<2>(GetParam());
    //
    //        AntennaTelemetry telemetry;
    //    }

    INSTANTIATE_TEST_CASE_P(AntennaTelemetryDeploymentStatus,
        AntennaTelemetryTest,
        testing::Values(std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, 0, 0x01),
            std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, 0, 0x02),
            std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, 0, 0x04),
            std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, 0, 0x08),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, 0, 0x10),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, 0, 0x20),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, 0, 0x40),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, 0, 0x80),

            std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA1_ID, ~0x01, 0xff),
            std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA2_ID, ~0x02, 0xff),
            std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA3_ID, ~0x04, 0xff),
            std::make_tuple(ANTENNA_PRIMARY_CHANNEL, ANTENNA4_ID, ~0x08, 0xff),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA1_ID, ~0x10, 0xff),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA2_ID, ~0x20, 0xff),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA3_ID, ~0x40, 0xff),
            std::make_tuple(ANTENNA_BACKUP_CHANNEL, ANTENNA4_ID, ~0x80, 0xff)), );
}
