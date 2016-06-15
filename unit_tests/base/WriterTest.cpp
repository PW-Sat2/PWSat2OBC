#include <algorithm>
#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/writer.h"
#include "system.h"

using testing::Eq;

void CheckBuffer(
    const uint8_t* current, const uint16_t currentLength, const uint8_t* expected, const uint16_t expectedLength)
{
    ASSERT_TRUE(std::equal(current, current + currentLength, expected, expected + expectedLength));
}

TEST(WriterTest, TestStatusNullBuffer)
{
    Writer writer;
    WriterInitialize(&writer, NULL, 1);
    ASSERT_FALSE(WriterStatus(&writer));
}

TEST(WriterTest, TestStatusZeroSizeBuffer)
{
    uint8_t array[1];
    Writer writer;
    WriterInitialize(&writer, array, 0);
    ASSERT_FALSE(WriterStatus(&writer));
}

TEST(WriterTest, TestStatusValidBuffer)
{
    Writer writer;
    uint8_t array[5];

    WriterInitialize(&writer, array, COUNT_OF(array));
    ASSERT_TRUE(WriterStatus(&writer));
}

TEST(WriterTest, TestBytesWrittenOnStart)
{
    Writer writer;
    uint8_t array[1];

    WriterInitialize(&writer, array, COUNT_OF(array));
    ASSERT_THAT(WriterGetDataLength(&writer), Eq(0));
}

TEST(WriterTest, TestWritingSingleByte)
{
    Writer writer;
    uint8_t array[1];
    const uint8_t expected[1] = {0x55};
    WriterInitialize(&writer, array, COUNT_OF(array));
    ASSERT_TRUE(WriterWriteByte(&writer, 0x55));
    ASSERT_TRUE(WriterStatus(&writer));
    CheckBuffer(array, WriterGetDataLength(&writer), expected, COUNT_OF(expected));
}

TEST(WriterTest, TestWritingSingleWordLE)
{
    Writer writer;
    uint8_t array[2];
    const uint8_t expected[2] = {0xAA, 0x55};
    WriterInitialize(&writer, array, COUNT_OF(array));
    ASSERT_TRUE(WriterWriteWordLE(&writer, 0x55aa));
    ASSERT_TRUE(WriterStatus(&writer));
    CheckBuffer(array, WriterGetDataLength(&writer), expected, COUNT_OF(expected));
}

TEST(WriterTest, TestWritingSingleDoubleWordLE)
{
    Writer writer;
    uint8_t array[4];
    const uint8_t expected[4] = {0xEE, 0x77, 0xAA, 0x55};
    WriterInitialize(&writer, array, COUNT_OF(array));
    ASSERT_TRUE(WriterWriteDoubleWordLE(&writer, 0x55aa77ee));
    ASSERT_TRUE(WriterStatus(&writer));
    CheckBuffer(array, WriterGetDataLength(&writer), expected, COUNT_OF(expected));
}

TEST(WriterTest, TestWritingArray)
{
    Writer writer;
    uint8_t input[] = {0x11, 0x22, 0x33};
    uint8_t array[4];
    const uint8_t expected[] = {0x11, 0x22, 0x33};
    WriterInitialize(&writer, array, COUNT_OF(array));
    ASSERT_TRUE(WriterWriteArray(&writer, input, COUNT_OF(input)));
    ASSERT_TRUE(WriterStatus(&writer));
    CheckBuffer(array, WriterGetDataLength(&writer), expected, COUNT_OF(expected));
}

TEST(WriterTest, TestWritingSingleByteBeyondEnd)
{
    Writer writer;
    uint8_t array[1];
    WriterInitialize(&writer, array, COUNT_OF(array));
    WriterWriteByte(&writer, 0x55);
    ASSERT_FALSE(WriterWriteByte(&writer, 0x55));
    ASSERT_FALSE(WriterStatus(&writer));
}

TEST(WriterTest, TestWritingSingleWordLEBeyondEnd)
{
    Writer writer;
    uint8_t array[1];
    WriterInitialize(&writer, array, COUNT_OF(array));
    ASSERT_FALSE(WriterWriteWordLE(&writer, 0x55aa));
    ASSERT_FALSE(WriterStatus(&writer));
}

TEST(WriterTest, TestWritingSingleDoubleWordLEBeyondEnd)
{
    Writer writer;
    uint8_t array[3];
    WriterInitialize(&writer, array, COUNT_OF(array));
    ASSERT_FALSE(WriterWriteDoubleWordLE(&writer, 0x55aa77ee));
    ASSERT_FALSE(WriterStatus(&writer));
}

TEST(WriterTest, TestWritingArrayBeyondEnd)
{
    Writer writer;
    uint8_t input[] = {0x11, 0x22, 0x33};
    uint8_t array[2];
    WriterInitialize(&writer, array, COUNT_OF(array));
    ASSERT_FALSE(WriterWriteArray(&writer, input, COUNT_OF(input)));
    ASSERT_FALSE(WriterStatus(&writer));
}

TEST(WriterTest, TestPositionIsAdvanced)
{
    Writer writer;
    uint8_t input[] = {0x11, 0x22, 0x33};
    uint8_t array[12];
    const uint8_t expected[] = {0x55, 0xAA, 0x55, 0xEE, 0x77, 0xAA, 0x55, 0x11, 0x22, 0x33};
    WriterInitialize(&writer, array, COUNT_OF(array));
    WriterWriteByte(&writer, 0x55);
    WriterWriteWordLE(&writer, 0x55aa);
    WriterWriteDoubleWordLE(&writer, 0x55aa77ee);
    WriterWriteArray(&writer, input, COUNT_OF(input));
    CheckBuffer(array, WriterGetDataLength(&writer), expected, COUNT_OF(expected));
}

TEST(WriterTest, TestBytesWritten)
{
    Writer writer;
    uint8_t array[12];
    WriterInitialize(&writer, array, COUNT_OF(array));
    WriterWriteByte(&writer, 0x55);
    WriterWriteWordLE(&writer, 0x55aa);
    WriterWriteDoubleWordLE(&writer, 0x55aa77ee);
    ASSERT_THAT(WriterGetDataLength(&writer), Eq(7));
}
