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
namespace
{
    TEST(DownlinkFrameTest, ShouldBuildProperDownlinkFrame)
    {
        auto apid = DownlinkAPID::Operation;
        uint32_t seq = 0x1DB55;

        DownlinkFrame frame(apid, seq);

        frame.PayloadWriter().WriteByte(0x42);

        ASSERT_THAT(frame.Frame().length(), Eq(4));
        ASSERT_THAT(frame.Frame()[0], Eq(0x42));
        ASSERT_THAT(frame.Frame()[1], Eq(0xD5));
        ASSERT_THAT(frame.Frame()[2], Eq(0x76));
        ASSERT_THAT(frame.Frame()[3], Eq(0x42));
    }

    TEST(DownlinkFrameTest, ShouldPreventBuildingTooBigFrame)
    {
        DownlinkFrame frame(DownlinkAPID::Telemetry, 1);

        array<uint8_t, DownlinkFrame::MaxPayloadSize> payload;
        payload.fill(0xAA);

        ASSERT_THAT(frame.PayloadWriter().WriteArray(payload), Eq(true));
        ASSERT_THAT(frame.PayloadWriter().WriteByte(0xAA), Eq(false));
    }
}
