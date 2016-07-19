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
    geometry.baseOffset = 0;

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
    geometry.baseOffset = 0;

    NANDCalculateGeometry(&geometry);

    uint8_t dataBuffer[1025];
    uint8_t spareBuffer[17];

    NANDOperation op;
    op.offset = 0x100;
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
    geometry.baseOffset = 0;

    NANDCalculateGeometry(&geometry);

    uint8_t dataBuffer[1025];
    uint8_t spareBuffer[17];

    NANDOperation op;
    op.offset = 0x100;
    op.dataSize = 1025;
    op.dataBuffer = dataBuffer;
    op.spareSize = 17;
    op.spareBuffer = spareBuffer;

    NANDOperationSlice slice[] = {
        NANDGetOperationSlice(&geometry, &op, 0), NANDGetOperationSlice(&geometry, &op, 1), NANDGetOperationSlice(&geometry, &op, 2)};

    ASSERT_THAT(Hex(slice[0].offset), Eq(Hex(0x100)));
    ASSERT_THAT(slice[0].dataSize, Eq(512));
    ASSERT_THAT(slice[0].dataBuffer, Eq(dataBuffer));
    ASSERT_THAT(slice[0].spareSize, Eq(16));
    ASSERT_THAT(slice[0].spareBuffer, Eq(spareBuffer));

    ASSERT_THAT(Hex(slice[1].offset), Eq(Hex(0x100 + 512)));
    ASSERT_THAT(slice[1].dataSize, Eq(512));
    ASSERT_THAT(slice[1].dataBuffer, Eq(dataBuffer + 512));
    ASSERT_THAT(slice[1].spareSize, Eq(1));
    ASSERT_THAT(slice[1].spareBuffer, Eq(spareBuffer + 16));

    ASSERT_THAT(Hex(slice[2].offset), Eq(Hex(0x100 + 1024)));
    ASSERT_THAT(slice[2].dataSize, Eq(1));
    ASSERT_THAT(slice[2].dataBuffer, Eq(dataBuffer + 1024));
    ASSERT_THAT(slice[2].spareSize, Eq(0));
    ASSERT_THAT(slice[2].spareBuffer, testing::IsNull());
}

class ShouldCalculatePageOffset : public testing::TestWithParam<std::tuple<uint32_t, uint32_t>>
{
};

TEST_P(ShouldCalculatePageOffset, ShouldCalculatePageOffset)
{
    NANDGeometry geometry;
    geometry.pageSize = 512;
    geometry.pagesPerBlock = 32;
    geometry.spareAreaPerPage = 16;
    geometry.pagesPerChunk = 2;
    geometry.baseOffset = 0x1000;

    NANDCalculateGeometry(&geometry);

    uint32_t chunkNo = std::get<0>(GetParam());
    uint32_t expectedAddress = std::get<1>(GetParam());

    auto address = NANDPageOffsetFromChunk(&geometry, chunkNo);

    ASSERT_EQ(Hex(address), Hex(expectedAddress));
}

INSTANTIATE_TEST_CASE_P(ShouldCalculatePageOffset,
    ShouldCalculatePageOffset,
    Values(std::make_tuple<uint32_t, uint32_t>(0, 0x1000),
                            std::make_tuple<uint32_t, uint32_t>(1, 0x1400),
                            std::make_tuple<uint32_t, uint32_t>(17, 0x5400)));

class ShouldCalculateBlockOffset : public testing::TestWithParam<std::tuple<uint32_t, uint32_t>>
{
};

TEST_P(ShouldCalculateBlockOffset, ShouldCalculateBlockOffset)
{
    NANDGeometry geometry;
    geometry.pageSize = 512;
    geometry.pagesPerBlock = 32;
    geometry.spareAreaPerPage = 16;
    geometry.pagesPerChunk = 2;
    geometry.baseOffset = 0x1000;

    NANDCalculateGeometry(&geometry);

    uint32_t blockNo = std::get<0>(GetParam());
    uint32_t expectedAddress = std::get<1>(GetParam());

    auto offset = NANDBlockOffset(&geometry, blockNo);

    ASSERT_EQ(Hex(offset), Hex(expectedAddress));
}

INSTANTIATE_TEST_CASE_P(ShouldCalculateBlockOffset,
    ShouldCalculateBlockOffset,
    Values(std::make_tuple<uint32_t, uint32_t>(0, 0x1000), std::make_tuple<uint32_t, uint32_t>(1, 0x5000)));
