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

TEST(NANDGeometryTest, ShouldCalculateAffectedPagesCount)
{
    NANDGeometry geometry;
    geometry.pageSize = 512;
    geometry.pagesPerBlock = 32;
    geometry.spareAreaPerPage = 16;
    geometry.pagesPerChunk = 2;
    geometry.baseAddress = 0x80000000UL;

    NANDCalculateGeometry(&geometry);

    uint8_t dataBuffer[1025];
    uint8_t spareBuffer[17];

    NANDOperation op;
    op.baseAddress = 0x80000000UL;
    op.dataSize = 1025;
    op.dataBuffer = dataBuffer;
    op.spareSize = 17;
    op.spareBuffer = spareBuffer;

    uint16_t pagesCount = NANDAffectedPagesCount(&geometry, &op);

    ASSERT_THAT(pagesCount, Eq(3));
}

TEST(NANDGeometryTest, ShouldCalculateRangesOfMultipageOperation2)
{
    NANDGeometry geometry;
    geometry.pageSize = 512;
    geometry.pagesPerBlock = 32;
    geometry.spareAreaPerPage = 16;
    geometry.pagesPerChunk = 2;
    geometry.baseAddress = 0x80000000UL;

    NANDCalculateGeometry(&geometry);

    uint8_t dataBuffer[1025];
    uint8_t spareBuffer[17];

    NANDOperation op;
    op.baseAddress = 0x80000000UL;
    op.dataSize = 1025;
    op.dataBuffer = dataBuffer;
    op.spareSize = 17;
    op.spareBuffer = spareBuffer;

    NANDOperationSlice slice[] = {NANDGetOperationSlice(&geometry, &op, 0),
        NANDGetOperationSlice(&geometry, &op, 1),
        NANDGetOperationSlice(&geometry, &op, 2)};

    ASSERT_THAT(Hex(slice[0].baseAddress), Eq(Hex(0x80000000UL)));
    ASSERT_THAT(slice[0].dataSize, Eq(512));
    ASSERT_THAT(slice[0].dataBuffer, Eq(dataBuffer));
    ASSERT_THAT(slice[0].spareSize, Eq(16));
    ASSERT_THAT(slice[0].spareBuffer, Eq(spareBuffer));

    ASSERT_THAT(Hex(slice[1].baseAddress), Eq(Hex(0x80000000UL + 512)));
    ASSERT_THAT(slice[1].dataSize, Eq(512));
    ASSERT_THAT(slice[1].dataBuffer, Eq(dataBuffer + 512));
    ASSERT_THAT(slice[1].spareSize, Eq(1));
    ASSERT_THAT(slice[1].spareBuffer, Eq(spareBuffer + 16));

    ASSERT_THAT(Hex(slice[2].baseAddress), Eq(Hex(0x80000000UL + 1024)));
    ASSERT_THAT(slice[2].dataSize, Eq(1));
    ASSERT_THAT(slice[2].dataBuffer, Eq(dataBuffer + 1024));
    ASSERT_THAT(slice[2].spareSize, Eq(0));
    ASSERT_THAT(slice[2].spareBuffer, testing::IsNull());
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
