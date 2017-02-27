#include <algorithm>
#include <array>
#include <cstdint>
#include "gtest/gtest.h"
#include "gmock/gmock-generated-matchers.h"
#include "gmock/gmock-matchers.h"
#include "base/writer.h"
#include "telecommunication/FrameContentWriter.hpp"

using testing::Eq;
using telecommunication::downlink::FrameContentWriter;
using telecommunication::downlink::FieldId;

static void CheckBuffer(const uint8_t* current, const uint16_t currentLength, const uint8_t* expected, const uint16_t expectedLength)
{
    ASSERT_TRUE(std::equal(current, current + currentLength, expected, expected + expectedLength));
}

TEST(FrameContentsWriterTest, TestWriteByte)
{
    uint8_t array[2];
    const uint8_t expected[] = {0x01, 0x55};
    Writer writer(array);
    FrameContentWriter frameWriter(writer);
    frameWriter.WriteByte(FieldId::TimeStamp, 0x55);
    CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
}

TEST(FrameContentsWriterTest, TestWriteWordLE)
{
    uint8_t array[3];
    const uint8_t expected[] = {0x01, 0xaa, 0x55};
    Writer writer(array);
    FrameContentWriter frameWriter(writer);
    frameWriter.WriteWordLE(FieldId::TimeStamp, 0x55aa);
    CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
}

TEST(FrameContentsWriterTest, TestWriteDWordLE)
{
    uint8_t array[5];
    const uint8_t expected[] = {0x01, 0xee, 0x77, 0xaa, 0x55};
    Writer writer(array);
    FrameContentWriter frameWriter(writer);
    frameWriter.WriteDoubleWordLE(FieldId::TimeStamp, 0x55aa77ee);
    CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
}

TEST(FrameContentsWriterTest, TestWriteQWordLE)
{
    uint8_t array[9];
    const uint8_t expected[] = {0x00, 0xee, 0x77, 0xcc, 0x66, 0xaa, 0x55, 0x88, 0x44};
    Writer writer(array);
    FrameContentWriter frameWriter(writer);
    frameWriter.WriteQuadWordLE(FieldId::None, 0x448855aa66cc77ee);
    CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
}

TEST(FrameContentsWriterTest, TestReset)
{
    uint8_t array[9];
    const uint8_t expected[] = {0x00, 0xee, 0x77, 0xcc, 0x66, 0xaa, 0x55, 0x88, 0x44};
    Writer writer(array);
    FrameContentWriter frameWriter(writer);
    frameWriter.WriteDoubleWordLE(FieldId::TimeStamp, 0x55aa77ee);
    frameWriter.Reset();
    frameWriter.WriteQuadWordLE(FieldId::None, 0x448855aa66cc77ee);
    CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
}
