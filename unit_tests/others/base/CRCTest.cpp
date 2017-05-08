#include <utility>
#include <vector>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/crc.h"
#include "utils.hpp"

using testing::Eq;
using testing::Values;

class CRCTest : public testing::TestWithParam<std::pair<std::uint16_t, std::vector<std::uint8_t>>>
{
};

TEST_P(CRCTest, ShouldCalculateProperly)
{
    auto expected = std::get<0>(GetParam());
    auto input = std::get<1>(GetParam());

    auto result = CRC_calc(&*input.begin(), &*input.end());

    ASSERT_THAT(Hex(result), Eq(Hex(expected)));
}

static CRCTest::ParamType Case(CRCTest::ParamType::first_type expected, CRCTest::ParamType::second_type input)
{
    return {expected, input};
}

INSTANTIATE_TEST_CASE_P(CRC,
    CRCTest,
    Values(                                                                                                                  //
        Case(0x686D, {6, 70, 57, 188, 173, 228, 22, 108, 7, 55, 129, 6, 50, 166, 139, 56, 150, 207, 1, 206}),                //
        Case(0x4766, {87, 39, 245, 86, 23, 24, 216, 154, 206, 186, 137, 249, 96, 141, 212, 158, 220, 147, 180, 11, 58, 74}), //
        Case(0x5A77, {59, 25}),                                                                                              //
        Case(0x2E68, {162, 93, 94, 53, 68, 239, 165, 155}),                                                                  //
        Case(0xE0E6, {186, 41, 97, 253, 163}),                                                                               //
        Case(0x4F9, {175, 215, 198, 58, 8, 80, 68, 54, 241, 224, 80, 167, 101, 234, 117, 67, 63}),                           //
        Case(0x2E4D, {67, 149, 229, 102, 56, 255, 130, 23, 12, 28, 160, 202, 108, 16, 97, 255, 135}),                        //
        Case(0xD735,
            {220,
                2,
                184,
                174,
                137,
                68,
                164,
                28,
                111,
                116,
                244,
                224,
                67,
                128,
                45,
                233,
                222,
                76,
                163,
                155,
                39,
                195,
                138,
                199,
                135,
                0,
                82,
                4,
                237,
                224,
                212}),                                                                      //
        Case(0x586B, {14, 224, 242, 21, 124, 17, 194, 196, 32}),                            //
        Case(0x426, {140, 67, 223, 108, 54, 138, 186, 51, 79, 254, 166, 112, 132, 30, 57}), //

        Case(0x0000, {}) //
        ), );
