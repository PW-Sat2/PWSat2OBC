
#include <array>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "redundancy.hpp"

using testing::Test;
using testing::Eq;

using std::uint8_t;
using std::uint32_t;
using gsl::span;

using namespace redundancy;

class RedundancyTest : public Test
{
};

TEST_F(RedundancyTest, ShouldFixSingleByte)
{
    ASSERT_THAT(Correct(0b0000000, 0b0000000, 0b0000000), Eq(0b0000000));
    ASSERT_THAT(Correct(0b0000001, 0b0000000, 0b0000000), Eq(0b0000000));
    ASSERT_THAT(Correct(0b0000000, 0b0000001, 0b0000000), Eq(0b0000000));
    ASSERT_THAT(Correct(0b0000000, 0b0000000, 0b0000001), Eq(0b0000000));
    ASSERT_THAT(Correct(0b0000001, 0b0000001, 0b0000000), Eq(0b0000001));
    ASSERT_THAT(Correct(0b0000001, 0b0000000, 0b0000001), Eq(0b0000001));
    ASSERT_THAT(Correct(0b0000000, 0b0000001, 0b0000001), Eq(0b0000001));
    ASSERT_THAT(Correct(0b0000001, 0b0000001, 0b0000001), Eq(0b0000001));

    ASSERT_THAT(Correct(0b0010000, 0b0010000, 0b0010000), Eq(0b0010000));
    ASSERT_THAT(Correct(0b0000001, 0b0010000, 0b0010000), Eq(0b0010000));
    ASSERT_THAT(Correct(0b0010000, 0b0010001, 0b0000000), Eq(0b0010000));
    ASSERT_THAT(Correct(0b1010101, 0b1110000, 0b0010001), Eq(0b1010001));
}

TEST_F(RedundancyTest, ShouldAllBytesInArray)
{
    uint8_t array1[] = {0b10101010, 0b1010101, 0b1010101};
    uint8_t array2[] = {0b11001100, 0b1110000, 0b1110000};
    uint8_t array3[] = {0b11110000, 0b0010001, 0b0010001};

    uint8_t expect[] = {0b11101000, 0b1010001, 0b1010001};

    auto b1 = gsl::span<uint8_t>(array1);
    auto b2 = gsl::span<uint8_t>(array2);
    auto b3 = gsl::span<uint8_t>(array3);

    ASSERT_THAT(CorrectBuffer(b1, b2, b3), Eq(true));
    ASSERT_THAT(b1, Eq(gsl::span<uint8_t>(expect)));
}

TEST_F(RedundancyTest, Voter)
{
    ASSERT_THAT(Vote<uint8_t>(1, 1, 1), Eq(Some<uint8_t>(1)));
    ASSERT_THAT(Vote<uint8_t>(1, 1, 2), Eq(Some<uint8_t>(1)));
    ASSERT_THAT(Vote<uint8_t>(1, 2, 1), Eq(Some<uint8_t>(1)));
    ASSERT_THAT(Vote<uint8_t>(2, 1, 1), Eq(Some<uint8_t>(1)));
    ASSERT_THAT(Vote<uint8_t>(1, 2, 2), Eq(Some<uint8_t>(2)));
    ASSERT_THAT(Vote<uint8_t>(1, 2, 3), Eq(None<uint8_t>()));
}
