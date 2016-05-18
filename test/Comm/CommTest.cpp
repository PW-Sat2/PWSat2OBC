#include "gmock/gmock-matchers.h"
#include "gmock/gmock.h"
#include "system.h"
#include "gtest/gtest.h"
#include <string>

using testing::Eq;
using testing::StrEq;

struct Frame {
	uint16_t size;
	uint16_t doppler;
	uint16_t rssi;
	uint8_t contents[255];
};

TEST(CommTest, ParseReceivedFrame)
{
	uint8_t raw[] = {3, 0, 44, 1, 64, 1, 65, 66, 67, 0};
	Frame * frame = (Frame*)raw;

	ASSERT_THAT(frame->size, Eq(3));
	ASSERT_THAT(frame->doppler, Eq(300));
	ASSERT_THAT(frame->rssi, Eq(320));
	ASSERT_THAT((char*)frame->contents, StrEq("ABC"));
}
