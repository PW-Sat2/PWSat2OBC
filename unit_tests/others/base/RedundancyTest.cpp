
#include <array>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "redundancy.hpp"

using testing::Test;
using testing::Eq;
using testing::ElementsAreArray;

using std::uint8_t;
using std::uint32_t;
using gsl::span;

using namespace redundancy;

TEST(RedundancyTest3, ShouldFixSingleByte)
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

TEST(RedundancyTest3, ShouldAllBytesInArray)
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

TEST(RedundancyTest3, Voter)
{
    ASSERT_THAT(Vote<uint8_t>(1, 1, 1), Eq(Some<uint8_t>(1)));
    ASSERT_THAT(Vote<uint8_t>(1, 1, 2), Eq(Some<uint8_t>(1)));
    ASSERT_THAT(Vote<uint8_t>(1, 2, 1), Eq(Some<uint8_t>(1)));
    ASSERT_THAT(Vote<uint8_t>(2, 1, 1), Eq(Some<uint8_t>(1)));
    ASSERT_THAT(Vote<uint8_t>(1, 2, 2), Eq(Some<uint8_t>(2)));
    ASSERT_THAT(Vote<uint8_t>(1, 2, 3), Eq(None<uint8_t>()));
}

TEST(RedundancyTest5, ShouldFixSingleByte)
{
    ASSERT_THAT(Correct(0b0, 0b0, 0b0, 0b0, 0b0), Eq(0));
    ASSERT_THAT(Correct(0b0, 0b0, 0b0, 0b0, 0b1), Eq(0));
    ASSERT_THAT(Correct(0b0, 0b0, 0b0, 0b1, 0b0), Eq(0));
    ASSERT_THAT(Correct(0b0, 0b0, 0b0, 0b1, 0b1), Eq(0));
    ASSERT_THAT(Correct(0b0, 0b0, 0b1, 0b0, 0b0), Eq(0));
    ASSERT_THAT(Correct(0b0, 0b0, 0b1, 0b0, 0b1), Eq(0));
    ASSERT_THAT(Correct(0b0, 0b0, 0b1, 0b1, 0b0), Eq(0));
    ASSERT_THAT(Correct(0b0, 0b0, 0b1, 0b1, 0b1), Eq(1));
    ASSERT_THAT(Correct(0b0, 0b1, 0b0, 0b0, 0b0), Eq(0));
    ASSERT_THAT(Correct(0b0, 0b1, 0b0, 0b0, 0b1), Eq(0));
    ASSERT_THAT(Correct(0b0, 0b1, 0b0, 0b1, 0b0), Eq(0));
    ASSERT_THAT(Correct(0b0, 0b1, 0b0, 0b1, 0b1), Eq(1));
    ASSERT_THAT(Correct(0b0, 0b1, 0b1, 0b0, 0b0), Eq(0));
    ASSERT_THAT(Correct(0b0, 0b1, 0b1, 0b0, 0b1), Eq(1));
    ASSERT_THAT(Correct(0b0, 0b1, 0b1, 0b1, 0b0), Eq(1));
    ASSERT_THAT(Correct(0b0, 0b1, 0b1, 0b1, 0b1), Eq(1));
    ASSERT_THAT(Correct(0b1, 0b0, 0b0, 0b0, 0b0), Eq(0));
    ASSERT_THAT(Correct(0b1, 0b0, 0b0, 0b0, 0b1), Eq(0));
    ASSERT_THAT(Correct(0b1, 0b0, 0b0, 0b1, 0b0), Eq(0));
    ASSERT_THAT(Correct(0b1, 0b0, 0b0, 0b1, 0b1), Eq(1));
    ASSERT_THAT(Correct(0b1, 0b0, 0b1, 0b0, 0b0), Eq(0));
    ASSERT_THAT(Correct(0b1, 0b0, 0b1, 0b0, 0b1), Eq(1));
    ASSERT_THAT(Correct(0b1, 0b0, 0b1, 0b1, 0b0), Eq(1));
    ASSERT_THAT(Correct(0b1, 0b0, 0b1, 0b1, 0b1), Eq(1));
    ASSERT_THAT(Correct(0b1, 0b1, 0b0, 0b0, 0b0), Eq(0));
    ASSERT_THAT(Correct(0b1, 0b1, 0b0, 0b0, 0b1), Eq(1));
    ASSERT_THAT(Correct(0b1, 0b1, 0b0, 0b1, 0b0), Eq(1));
    ASSERT_THAT(Correct(0b1, 0b1, 0b0, 0b1, 0b1), Eq(1));
    ASSERT_THAT(Correct(0b1, 0b1, 0b1, 0b0, 0b0), Eq(1));
    ASSERT_THAT(Correct(0b1, 0b1, 0b1, 0b0, 0b1), Eq(1));
    ASSERT_THAT(Correct(0b1, 0b1, 0b1, 0b1, 0b0), Eq(1));
    ASSERT_THAT(Correct(0b1, 0b1, 0b1, 0b1, 0b1), Eq(1));
}

TEST(RedundancyTest5, ShouldAllBytesInArray)
{
    std::array<uint8_t, 3> array1{0b10101010, 0b01010101, 0b11010101};
    std::array<uint8_t, 3> array2{0b11001100, 0b01110000, 0b01110000};
    std::array<uint8_t, 3> array3{0b11110000, 0b00010001, 0b10010001};
    std::array<uint8_t, 3> array4{0b00100000, 0b00111101, 0b10110011};
    std::array<uint8_t, 3> array5{0b10110010, 0b11001110, 0b11110000};

    uint8_t expect[] = {0b10100000, 0b01010101, 0b11110001};

    std::array<gsl::span<const uint8_t>, 5> spans{array1, array2, array3, array4, array5};

    std::array<uint8_t, 3> result{0};

    auto r = CorrectBuffer(gsl::make_span(result), spans);

    ASSERT_THAT(r, Eq(true));
    ASSERT_THAT(result, ElementsAreArray(expect, 3));
}
