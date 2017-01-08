#include <algorithm>
#include <array>
#include <cstdint>
#include <gsl/span>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "base/reader.h"
#include "comm/CommDriver.hpp"
#include "system.h"
#include "telecommand_handling/uplink.h"

using std::uint8_t;
using gsl::span;
using testing::Test;
using testing::Eq;
using testing::ElementsAre;
using std::array;
using telecommands::handling::IDecodeTelecommand;
using telecommands::handling::DecodeTelecommandResult;
using telecommands::handling::DecodeTelecommandFailureReason;
using telecommands::UplinkProtocol;

class UplinkFrameDecoderTest : public Test
{
  protected:
    UplinkFrameDecoderTest();

    UplinkProtocol _proto;
};

UplinkFrameDecoderTest::UplinkFrameDecoderTest() : _proto(0xAABBCCDD)
{
}

TEST_F(UplinkFrameDecoderTest, ShouldDecodeValidFrame)
{
    array<uint8_t, 10> frame{0xAA, 0xBB, 0xCC, 0xDD, 0x11, 10, 11, 12, 13, 14};

    auto r = this->_proto.Decode(frame);

    ASSERT_THAT(r.IsSuccess, Eq(true));
    ASSERT_THAT(r.CommandCode, Eq(0x11));
    ASSERT_THAT(r.Parameters, ElementsAre(10, 11, 12, 13, 14));
}

TEST_F(UplinkFrameDecoderTest, ShouldDecodeValidFrameOfMaximumSize)
{
    array<uint8_t, devices::comm::MaxUplinkFrameSize - 5> parameters;

    for (decltype(parameters.size()) i = 0; i < parameters.size(); i++)
    {
        parameters[i] = i;
    }

    array<uint8_t, devices::comm::MaxUplinkFrameSize> frame;
    frame[0] = 0xAA;
    frame[1] = 0xBB;
    frame[2] = 0xCC;
    frame[3] = 0xDD;
    frame[4] = 0x22;

    std::copy(parameters.begin(), parameters.end(), frame.begin() + 5);

    auto r = this->_proto.Decode(frame);

    ASSERT_THAT(r.IsSuccess, Eq(true));
    ASSERT_THAT(r.CommandCode, Eq(0x22));
    ASSERT_THAT(r.Parameters, Eq(span<const uint8_t>(parameters)));
}

TEST_F(UplinkFrameDecoderTest, ShouldNotDecodeTooShortFrame)
{
    array<uint8_t, 2> frame{1, 2};

    auto r = this->_proto.Decode(frame);

    ASSERT_THAT(r.IsSuccess, Eq(false));
    ASSERT_THAT(r.FailureReason, Eq(DecodeTelecommandFailureReason::MalformedFrame));
}

TEST_F(UplinkFrameDecoderTest, ShouldDiscardFrameWithoutValidSecurityCode)
{
    array<uint8_t, 10> frame{0xBB, 0xBB, 0xCC, 0xDD, 0x11, 10, 11, 12, 13, 14};

    auto r = this->_proto.Decode(frame);

    ASSERT_THAT(r.IsSuccess, Eq(false));
    ASSERT_THAT(r.FailureReason, Eq(DecodeTelecommandFailureReason::InvalidSecurityCode));
}
