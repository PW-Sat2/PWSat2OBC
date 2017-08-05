#include <algorithm>
#include "gtest/gtest.h"
#include "gmock/gmock-generated-matchers.h"
#include "gmock/gmock-matchers.h"
#include "base/reader.h"
#include "base/writer.h"

using testing::Eq;
namespace
{
    void CheckBuffer(const uint8_t* current, const uint16_t currentLength, const uint8_t* expected, const uint16_t expectedLength)
    {
        ASSERT_TRUE(std::equal(current, current + currentLength, expected, expected + expectedLength));
    }

    TEST(WriterTest, TestStatusNullBuffer)
    {
        Writer writer;
        ASSERT_FALSE(writer.Status());
    }

    TEST(WriterTest, TestStatusZeroSizeBuffer)
    {
        uint8_t array[1];
        Writer writer(gsl::make_span(array, 0));
        ASSERT_FALSE(writer.Status());
    }

    TEST(WriterTest, TestStatusValidBuffer)
    {
        uint8_t array[5];
        Writer writer(array);
        ASSERT_TRUE(writer.Status());
    }

    TEST(WriterTest, TestBytesWrittenOnStart)
    {
        uint8_t array[1];
        Writer writer(array);
        ASSERT_THAT(writer.GetDataLength(), Eq(0));
    }

    TEST(WriterTest, TestWritingSingleByte)
    {
        uint8_t array[1];
        const uint8_t expected[1] = {0x55};
        Writer writer(array);
        ASSERT_TRUE(writer.WriteByte(0x55));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
    }

    TEST(WriterTest, TestWritingSingleWordLE)
    {
        uint8_t array[2];
        const uint8_t expected[2] = {0xAA, 0x55};
        Writer writer(array);
        ASSERT_TRUE(writer.WriteWordLE(0x55aa));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
    }

    TEST(WriterTest, TestWritingSingleWordBE)
    {
        uint8_t array[2];
        const uint8_t expected[2] = {0xAA, 0x55};
        Writer writer(array);
        ASSERT_TRUE(writer.WriteWordBE(0xAA55));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
    }

    TEST(WriterTest, TestWritingSignedSingleWordLE)
    {
        uint8_t array[12];
        Writer writer(array);

        const uint8_t expected[] = {0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x80, 0x68, 0xC5, 0x98, 0x3A};

        ASSERT_TRUE(writer.WriteSignedWordLE(0));
        ASSERT_TRUE(writer.WriteSignedWordLE(-1));
        ASSERT_TRUE(writer.WriteSignedWordLE(32767));
        ASSERT_TRUE(writer.WriteSignedWordLE(-32768));
        ASSERT_TRUE(writer.WriteSignedWordLE(-15000));
        ASSERT_TRUE(writer.WriteSignedWordLE(15000));

        ASSERT_TRUE(writer.Status());
        CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
    }

    TEST(WriterTest, TestWritingSingleDoubleWordLE)
    {
        uint8_t array[4];
        const uint8_t expected[4] = {0xEE, 0x77, 0xAA, 0x55};
        Writer writer(array);
        ASSERT_TRUE(writer.WriteDoubleWordLE(0x55aa77ee));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
    }

    TEST(WriterTest, TestWritingSignedDoubleWordLE)
    {
        uint8_t array[32];
        const uint8_t expected[32] = {0x0,
            0x0,
            0x0,
            0x0,
            0xFF,
            0xFF,
            0xFF,
            0xFF,
            0xFF,
            0x7F,
            0x0,
            0x0,
            0x0,
            0x80,
            0xFF,
            0xFF,
            0xFF,
            0xFF,
            0xFF,
            0x7F,
            0x0,
            0x0,
            0x0,
            0x80,
            0xD2,
            0x2,
            0x96,
            0x49,
            0x2E,
            0xFD,
            0x69,
            0xB6};
        Writer writer(array);
        ASSERT_TRUE(writer.WriteSignedDoubleWordLE(0));
        ASSERT_TRUE(writer.WriteSignedDoubleWordLE(-1));
        ASSERT_TRUE(writer.WriteSignedDoubleWordLE(32767));
        ASSERT_TRUE(writer.WriteSignedDoubleWordLE(-32768));
        ASSERT_TRUE(writer.WriteSignedDoubleWordLE(2147483647));
        ASSERT_TRUE(writer.WriteSignedDoubleWordLE(-2147483648));
        ASSERT_TRUE(writer.WriteSignedDoubleWordLE(1234567890));
        ASSERT_TRUE(writer.WriteSignedDoubleWordLE(-1234567890));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
    }

    TEST(WriterTest, TestWritingSingleQuadWordLE)
    {
        uint8_t array[8];
        const uint8_t expected[8] = {0xEE, 0x77, 0xAA, 0x55, 0xCC, 0x66, 0x88, 0x44};
        Writer writer(array);
        ASSERT_TRUE(writer.WriteQuadWordLE(0x448866CC55aa77eeull));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
    }

    TEST(WriterTest, TestWritingArray)
    {
        uint8_t input[] = {0x11, 0x22, 0x33};
        uint8_t array[4];
        const uint8_t expected[] = {0x11, 0x22, 0x33};
        Writer writer(array);
        ASSERT_TRUE(writer.WriteArray(input));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
    }

    TEST(WriterTest, TestWritingSingleByteBeyondEnd)
    {
        uint8_t array[1];
        Writer writer(array);
        writer.WriteByte(0x55);
        ASSERT_FALSE(writer.WriteByte(0x55));
        ASSERT_FALSE(writer.Status());
    }

    TEST(WriterTest, TestWritingSingleWordLEBeyondEnd)
    {
        uint8_t array[1];
        Writer writer(array);
        ASSERT_FALSE(writer.WriteWordLE(0x55aa));
        ASSERT_FALSE(writer.Status());
    }

    TEST(WriterTest, TestWritingSingleDoubleWordLEBeyondEnd)
    {
        uint8_t array[3];
        Writer writer(array);
        ASSERT_FALSE(writer.WriteDoubleWordLE(0x55aa77ee));
        ASSERT_FALSE(writer.Status());
    }

    TEST(WriterTest, TestWritingSingleQuadWordLEBeyondEnd)
    {
        uint8_t array[7];
        Writer writer(array);
        ASSERT_FALSE(writer.WriteQuadWordLE(0x448866CC55aa77eeull));
        ASSERT_FALSE(writer.Status());
    }

    TEST(WriterTest, TestWritingArrayBeyondEnd)
    {
        uint8_t input[] = {0x11, 0x22, 0x33};
        uint8_t array[2];
        Writer writer(array);
        ASSERT_FALSE(writer.WriteArray(input));
        ASSERT_FALSE(writer.Status());
    }

    TEST(WriterTest, TestPositionIsAdvanced)
    {
        uint8_t input[] = {0x11, 0x22, 0x33};
        uint8_t array[12];
        const uint8_t expected[] = {0x55, 0xAA, 0x55, 0xEE, 0x77, 0xAA, 0x55, 0x11, 0x22, 0x33};
        Writer writer(array);
        writer.WriteByte(0x55);
        writer.WriteWordLE(0x55aa);
        writer.WriteDoubleWordLE(0x55aa77ee);
        writer.WriteArray(input);
        CheckBuffer(array, writer.GetDataLength(), expected, sizeof(expected));
    }

    TEST(WriterTest, TestBytesWritten)
    {
        uint8_t array[12];
        Writer writer(array);
        writer.WriteByte(0x55);
        writer.WriteWordLE(0x55aa);
        writer.WriteDoubleWordLE(0x55aa77ee);
        ASSERT_THAT(writer.GetDataLength(), Eq(7));
    }

    TEST(WriterTest, TestUsedSpan)
    {
        uint8_t array[12];
        Writer writer(array);
        writer.WriteByte(0x55);
        writer.WriteByte(0x66);

        auto s = writer.Capture();

        ASSERT_THAT(s.length(), Eq(2));
        ASSERT_THAT(s, testing::ElementsAre(0x55, 0x66));
    }

    TEST(WriterTest, TestWriteLowerBytesBE)
    {
        uint32_t num = 0xAABBCCDD;

        uint8_t array[3];
        Writer writer(array);

        writer.WriteLowerBytesBE(num, 3);

        ASSERT_THAT(array[0], Eq(0xBB));
        ASSERT_THAT(array[1], Eq(0xCC));
        ASSERT_THAT(array[2], Eq(0xDD));
    }

    TEST(WriterTest, TestReserveBuffer)
    {
        uint8_t array[5];
        Writer writer(array);

        writer.WriteByte(1);
        auto b = writer.Reserve(3);
        b[0] = 2;
        b[1] = 3;
        b[2] = 4;
        writer.WriteByte(5);

        ASSERT_THAT(array, testing::ElementsAre(1, 2, 3, 4, 5));
    }

    TEST(WriterTest, TestFill)
    {
        uint8_t array[5];
        Writer writer(array);

        writer.WriteByte(1);
        writer.Fill(4);

        ASSERT_THAT(array, testing::ElementsAre(1, 4, 4, 4, 4));
    }

    TEST(WriterTest, TestReserveBufferOverflowingBuffer)
    {
        uint8_t array[3];
        Writer writer(array);

        auto b = writer.Reserve(5);

        ASSERT_THAT(b.size(), Eq(3));
        ASSERT_THAT(writer.Status(), Eq(true));

        writer.WriteByte(1);

        ASSERT_THAT(writer.Status(), Eq(false));
    }

    TEST(WriterTest, TestBCDRoundTrip)
    {
        for (std::uint16_t i = 0; i < 100; i++)
        {
            uint8_t buffer[1];
            Writer w(buffer);
            w.WriteByteBCD(i);

            Reader r(buffer);
            auto readBack = r.ReadByteBCD(0b11110000);

            ASSERT_THAT(readBack, Eq(i)) << "BCD roundtrip " << i;
        }
    }
}
