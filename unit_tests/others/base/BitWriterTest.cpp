#include <algorithm>
#include <cstring>
#include <limits>
#include "gtest/gtest.h"
#include "gmock/gmock-generated-matchers.h"
#include "gmock/gmock-matchers.h"
#include "base/BitWriter.hpp"
#include "utils.h"

namespace
{
    using testing::Eq;

    void CheckBuffer(gsl::span<std::uint8_t> current, gsl::span<const std::uint8_t> expected)
    {
        ASSERT_THAT(current.size(), Eq(expected.size()));
        ASSERT_THAT(std::memcmp(current.data(), expected.data(), current.size()), Eq(0));
    }

    TEST(BitWriterTest, TestStatusNullBuffer)
    {
        BitWriter writer;
        ASSERT_FALSE(writer.Status());
    }

    TEST(BitWriterTest, TestStatusZeroSizeBuffer)
    {
        uint8_t array[1];
        BitWriter writer(gsl::make_span(array, 0));
        ASSERT_FALSE(writer.Status());
    }

    TEST(BitWriterTest, TestStatusValidBuffer)
    {
        uint8_t array[5];
        BitWriter writer(array);
        ASSERT_TRUE(writer.Status());
    }

    TEST(BitWriterTest, TestBytesWrittenOnStart)
    {
        uint8_t array[1];
        BitWriter writer(array);
        ASSERT_THAT(writer.GetBitDataLength(), Eq(0u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(0u));
    }

    TEST(BitWriterTest, TestWritingSingleByte)
    {
        uint8_t array[1];
        const uint8_t expected[1] = {0x55};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(0x55, 8));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(8u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(0u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(1u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingSingleBit)
    {
        uint8_t array[1];
        const uint8_t expected[1] = {0x1};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(0x1, 1));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(1u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(1u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(1u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingBool)
    {
        uint8_t array[1];
        const uint8_t expected[1] = {0x1};
        BitWriter writer(array);
        ASSERT_TRUE(writer.Write(true));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(1u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(1u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(1u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingSeveralBits)
    {
        uint8_t array[1];
        const uint8_t expected[1] = {0x15};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(0x1, 1));
        ASSERT_TRUE(writer.WriteWord(0x0, 1));
        ASSERT_TRUE(writer.Write(true));
        ASSERT_TRUE(writer.WriteWord(0x0, 1));
        ASSERT_TRUE(writer.WriteWord(0x1, 1));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(5u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(5u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(1u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingFullWordNoBreaks)
    {
        uint8_t array[2];
        const uint8_t expected[] = {0xaa, 0x55};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(0x55aa, 16));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(16u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(0u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(2u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingWord)
    {
        uint8_t array[2];
        const uint8_t expected[] = {0xaa, 0x55};
        BitWriter writer(array);
        ASSERT_TRUE(writer.Write(static_cast<std::uint16_t>(0x55aa)));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(16u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(0u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(2u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingFullDoubleWordNoBreaks)
    {
        uint8_t array[4];
        const uint8_t expected[] = {0xee, 0x77, 0xaa, 0x55};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteDoubleWord(0x55aa77ee, 32));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(32u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(0u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(4u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingDoubleWord)
    {
        uint8_t array[4];
        const uint8_t expected[] = {0xee, 0x77, 0xaa, 0x55};
        BitWriter writer(array);
        ASSERT_TRUE(writer.Write(static_cast<std::uint32_t>(0x55aa77ee)));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(32u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(0u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(4u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingFullQuadWordNoBreaks)
    {
        uint8_t array[8];
        const uint8_t expected[] = {0x81, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteQuadWord(0xffeeddccbbaa9981ull, 64));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(64u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(0u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(8u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingQuadWord)
    {
        uint8_t array[8];
        const uint8_t expected[] = {0x81, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
        BitWriter writer(array);
        ASSERT_TRUE(writer.Write(0xffeeddccbbaa9981ull));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(64u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(0u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(8u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingFullWordWithBreaks)
    {
        uint8_t array[3];
        const uint8_t expected[] = {0x55, 0x11, 0x1};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.WriteWord(0x88aa, 16));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(17u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(1u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(3u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingFullDoubleWordWithBreaks)
    {
        uint8_t array[5];
        const uint8_t expected[] = {0xdd, 0xef, 0x54, 0x11, 0x1};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.WriteDoubleWord(0x88aa77ee, 32));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(33u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(1u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(5u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingFullQuadWordWithBreaks)
    {
        uint8_t array[9];
        const uint8_t expected[] = {0x03, 0x33, 0x55, 0x77, 0x99, 0xbb, 0xdd, 0xff, 0x1};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.WriteQuadWord(0xffeeddccbbaa9981ull, 64));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(65u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(1u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(9u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingWordZeroBits)
    {
        uint8_t array[3];
        const uint8_t expected[] = {0x1};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.WriteWord(0x88aa, 0));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(1u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(1u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(1u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingDoubleWordZeroBits)
    {
        uint8_t array[5];
        const uint8_t expected[] = {0x1};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.WriteDoubleWord(0x88aa77ee, 0));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(1u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(1u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(1u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingQuadWordZeroBits)
    {
        uint8_t array[9];
        const uint8_t expected[] = {0x1};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.WriteQuadWord(0xffeeddccbbaa9981ull, 0));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(1u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(1u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(1u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingWordAfterFailureWithSpace)
    {
        uint8_t array[2];
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_FALSE(writer.WriteWord(0x88aa, 16));
        ASSERT_FALSE(writer.WriteWord(0x88aa, 1));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(1u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(1u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(1u));
        ASSERT_FALSE(writer.Status());
    }

    TEST(BitWriterTest, TestWritingDoubleWordAfterFailureWithSpace)
    {
        uint8_t array[4];
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_FALSE(writer.WriteDoubleWord(0x88aa77ee, 32));
        ASSERT_FALSE(writer.WriteDoubleWord(0x88aa77ee, 1));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(1u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(1u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(1u));
        ASSERT_FALSE(writer.Status());
    }

    TEST(BitWriterTest, TestWritingQuadWordAfterFailureWithSpace)
    {
        uint8_t array[8];
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_FALSE(writer.WriteQuadWord(0xffeeddccbbaa9981ull, 64));
        ASSERT_FALSE(writer.WriteQuadWord(0xffeeddccbbaa9981ull, 1));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(1u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(1u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(1u));
        ASSERT_FALSE(writer.Status());
    }

    TEST(BitWriterTest, TestWritingPartialWord)
    {
        uint8_t array[3];
        const uint8_t expected[] = {0x55, 0x51};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.WriteWord(0xe8aa, 14));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(15u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(7u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(2u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingPartialDoubleWord)
    {
        uint8_t array[5];
        const uint8_t expected[] = {0xd5, 0x6f};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.WriteDoubleWord(0x88aaf7ea, 14));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(15u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(7u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(2u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingPartialQuadWord)
    {
        uint8_t array[9];
        const uint8_t expected[] = {0xd5, 0x6f};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.WriteQuadWord(0xffeeddccbbaaf7eaull, 14));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(15u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(7u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(2u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingHalfQuadWord)
    {
        uint8_t array[9];
        const uint8_t expected[] = {0xd5, 0xef, 0x55, 0x77};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.WriteQuadWord(0xffeeddccfbaaf7eaull, 30));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(31u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(7u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(4u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingMostOfQuadWord)
    {
        uint8_t array[9];
        const uint8_t expected[] = {0xd5, 0xef, 0x55, 0xf7, 0x99, 0x3b};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.WriteQuadWord(0xffeeddccfbaaf7eaull, 46));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(47u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(7u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(6u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestCaptureAfterFailure)
    {
        uint8_t array[1];
        BitWriter writer(array);
        writer.WriteWord(0xff, 7);
        writer.WriteWord(1, 2);
        ASSERT_FALSE(writer.Status());
        ASSERT_THAT(writer.Capture().empty(), Eq(true));
    }

    TEST(BitWriterTest, TestWritingPartialBitWord)
    {
        uint8_t array[3];
        const uint8_t expected[] = {0x55, 0x51};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.Write(BitValue<std::uint16_t, 14>(0xe8aa)));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(15u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(7u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(2u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingPartialDoubleBitWord)
    {
        uint8_t array[5];
        const uint8_t expected[] = {0xd5, 0xef, 0x55, 0x77};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.Write(BitValue<std::uint32_t, 30>(0xfbaaf7ea)));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(31u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(7u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(4u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }

    TEST(BitWriterTest, TestWritingPartialQuadBitWord)
    {
        uint8_t array[9];
        const uint8_t expected[] = {0xd5, 0xef, 0x55, 0xf7, 0x99, 0x3b};
        BitWriter writer(array);
        ASSERT_TRUE(writer.WriteWord(1, 1));
        ASSERT_TRUE(writer.Write(BitValue<std::uint64_t, 46>(0xffeeddccfbaaf7eaull)));
        ASSERT_THAT(writer.GetBitDataLength(), Eq(47u));
        ASSERT_THAT(writer.GetBitFraction(), Eq(7u));
        ASSERT_THAT(writer.GetByteDataLength(), Eq(6u));
        ASSERT_TRUE(writer.Status());
        CheckBuffer(writer.Capture(), gsl::make_span(expected));
    }
}
