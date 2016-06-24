#include <stdint.h>
#include <ios>
#include <tuple>
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "storage/nand.h"
#include "utils.hpp"

using testing::Eq;
using testing::Values;

TEST(NANDGeometryTest, ShouldCalculateBaseParameters)
{
    NANDGeometry geometry;
    geometry.pageSize = 512;
    geometry.pagesPerBlock = 32;
    geometry.spareAreaPerPage = 16;
    geometry.pagesPerChunk = 2;
    geometry.baseAddress = 0x80000000UL;

    NANDCalculateGeometry(&geometry);

    ASSERT_THAT(geometry.chunkSize, Eq(1024));
    ASSERT_THAT(geometry.blockSize, Eq(16384));
    ASSERT_THAT(geometry.chunksPerBlock, Eq(16));
}

class ShouldCalculatePageBaseAddress : public testing::TestWithParam<std::tuple<uint32_t, uint32_t>>
{
};

TEST_P(ShouldCalculatePageBaseAddress, ShouldCalculatePageBaseAddress)
{
    NANDGeometry geometry;
    geometry.pageSize = 512;
    geometry.pagesPerBlock = 32;
    geometry.spareAreaPerPage = 16;
    geometry.pagesPerChunk = 2;
    geometry.baseAddress = 0x80000000UL;

    NANDCalculateGeometry(&geometry);

    uint32_t chunkNo = std::get<0>(GetParam());
    uint32_t expectedAddress = std::get<1>(GetParam());

    auto address = NANDPageBaseAddressFromChunk(&geometry, chunkNo);

    ASSERT_EQ(Hex(address), Hex(expectedAddress));
}

INSTANTIATE_TEST_CASE_P(ShouldCalculatePageBaseAddress,
    ShouldCalculatePageBaseAddress,
    Values(std::make_tuple<uint32_t, uint32_t>(0, 0x80000000),
                            std::make_tuple<uint32_t, uint32_t>(1, 0x80000400),
                            std::make_tuple<uint32_t, uint32_t>(17, 0x80004400)));

class ShouldCalculateBlockBaseAddress : public testing::TestWithParam<std::tuple<uint32_t, uint32_t>>
{
};

TEST_P(ShouldCalculateBlockBaseAddress, ShouldCalculateBlockBaseAddress)
{
    NANDGeometry geometry;
    geometry.pageSize = 512;
    geometry.pagesPerBlock = 32;
    geometry.spareAreaPerPage = 16;
    geometry.pagesPerChunk = 2;
    geometry.baseAddress = 0x80000000UL;

    NANDCalculateGeometry(&geometry);

    uint32_t blockNo = std::get<0>(GetParam());
    uint32_t expectedAddress = std::get<1>(GetParam());

    auto address = NANDBlockBaseAddress(&geometry, blockNo);

    ASSERT_EQ(Hex(address), Hex(expectedAddress));
}

INSTANTIATE_TEST_CASE_P(ShouldCalculateBlockBaseAddress,
    ShouldCalculateBlockBaseAddress,
    Values(std::make_tuple<uint32_t, uint32_t>(0, 0x80000000), std::make_tuple<uint32_t, uint32_t>(1, 0x80004000)));
