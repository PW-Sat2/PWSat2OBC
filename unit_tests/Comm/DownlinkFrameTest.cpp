#include <array>
#include <cstdint>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/writer.h"
#include "comm/comm.hpp"
#include "system.h"
#include "telecommunication/downlink.h"
#include "utils.h"

using testing::Test;
using testing::Eq;
using std::array;
using std::uint8_t;
using std::uint32_t;
using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

TEST(DownlinkFrameTest, ShouldBuildProperDownlinkFrame)
{
    auto apid = DownlinkAPID::TelemetryShort;
    uint32_t seq = 0x30F0F;

    DownlinkFrame frame(apid, seq);

    frame.PayloadWriter().WriteByte(0x42);

    ASSERT_THAT(frame.Frame().length(), Eq(4));
    ASSERT_THAT(frame.Frame()[0], Eq(0b10101011));
    ASSERT_THAT(frame.Frame()[1], Eq(0x0F));
    ASSERT_THAT(frame.Frame()[2], Eq(0x0F));
    ASSERT_THAT(frame.Frame()[3], Eq(0x42));
}

TEST(DownlinkFrameTest, ShouldPreventBuildingTooBigFrame)
{
    DownlinkFrame frame(DownlinkAPID::TelemetryLong, 1);

    array<uint8_t, DownlinkFrame::MaxPayloadSize> payload;
    payload.fill(0xAA);

    ASSERT_THAT(frame.PayloadWriter().WriteArray(payload), Eq(true));
    ASSERT_THAT(frame.PayloadWriter().WriteByte(0xAA), Eq(false));
}
