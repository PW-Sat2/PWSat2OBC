#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/reader.h"
#include "system.h"

using testing::Eq;
using testing::ElementsAre;
namespace
{
    TEST(ReaderTest, TestDefaultCtor)
    {
        Reader reader;
        ASSERT_FALSE(reader.Status());
        ASSERT_THAT(reader.RemainingSize(), Eq(0));
    }

    TEST(ReaderTest, TestStatusNullBuffer)
    {
        gsl::span<const std::uint8_t> span;
        Reader reader(span);
        ASSERT_FALSE(reader.Status());
    }

    TEST(ReaderTest, TestStatusZeroSizeBuffer)
    {
        uint8_t array[1];
        Reader reader;
        reader.Initialize(gsl::span<const std::uint8_t>(array, 0));
        ASSERT_FALSE(reader.Status());
    }

    TEST(ReaderTest, TestCtorStatusValidBuffer)
    {
        uint8_t array[] = {0x55, 0xaa};

        Reader reader(array);
        ASSERT_TRUE(reader.Status());
        ASSERT_THAT(reader.RemainingSize(), Eq(2));
    }

    TEST(ReaderTest, TestStatusValidBuffer)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa};

        reader.Initialize(array);
        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestReadingSingleByte)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa};

        reader.Initialize(array);
        ASSERT_THAT(reader.ReadByte(), Eq(0x55));
        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestSkip)
    {
        uint8_t array[] = {0x55, 0xaa};
        Reader reader(array);
        ASSERT_THAT(reader.Skip(1), Eq(true));
        ASSERT_TRUE(reader.Status());
        ASSERT_THAT(reader.RemainingSize(), Eq(1));
    }

    TEST(ReaderTest, TestSkipOverTheLimit)
    {
        uint8_t array[] = {0x55, 0xaa};
        Reader reader(array);
        ASSERT_THAT(reader.Skip(3), Eq(false));
        ASSERT_FALSE(reader.Status());
    }

    TEST(ReaderTest, TestSkipToTheLimit)
    {
        uint8_t array[] = {0x55, 0xaa};
        Reader reader(array);
        ASSERT_THAT(reader.Skip(2), Eq(true));
        ASSERT_TRUE(reader.Status());
        ASSERT_THAT(reader.RemainingSize(), Eq(0));
    }

    TEST(ReaderTest, TestReadingSingleByteBeyondEnd)
    {
        Reader reader;
        uint8_t array[] = {0x55};

        reader.Initialize(array);
        reader.ReadByte();
        reader.ReadByte();
        ASSERT_FALSE(reader.Status());
    }

    TEST(ReaderTest, TestReadingWordLE)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa};

        reader.Initialize(array);
        ASSERT_THAT(reader.ReadWordLE(), Eq(0xaa55));
        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestReadingSignedWordLE)
    {
        uint8_t array[] = {0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x80, 0x68, 0xC5, 0x98, 0x3A};
        Reader reader;
        reader.Initialize(array);

        ASSERT_THAT(reader.ReadSignedWordLE(), Eq(0));
        ASSERT_THAT(reader.ReadSignedWordLE(), Eq(-1));
        ASSERT_THAT(reader.ReadSignedWordLE(), Eq(32767));
        ASSERT_THAT(reader.ReadSignedWordLE(), Eq(-32768));
        ASSERT_THAT(reader.ReadSignedWordLE(), Eq(-15000));
        ASSERT_THAT(reader.ReadSignedWordLE(), Eq(15000));
        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestReadingWordBE)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa};

        reader.Initialize(array);
        ASSERT_THAT(reader.ReadWordBE(), Eq(0x55aa));
        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestReadingWordLEBeyondEnd)
    {
        Reader reader;
        uint8_t array[] = {0x55};

        reader.Initialize(array);
        reader.ReadWordLE();
        ASSERT_FALSE(reader.Status());
    }

    TEST(ReaderTest, TestReadingWordBEBeyondEnd)
    {
        Reader reader;
        uint8_t array[] = {0x55};

        reader.Initialize(array);
        reader.ReadWordBE();
        ASSERT_FALSE(reader.Status());
    }

    TEST(ReaderTest, TestReadingDWordLE)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa, 0x77, 0xee};

        reader.Initialize(array);
        ASSERT_THAT(reader.ReadDoubleWordLE(), Eq(0xEE77AA55U));
        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestReadingSignedDWordLE)
    {
        uint8_t array[] = {0x0,
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
        Reader reader;
        reader.Initialize(array);

        ASSERT_THAT(reader.ReadSignedDoubleWordLE(), Eq(0));
        ASSERT_THAT(reader.ReadSignedDoubleWordLE(), Eq(-1));
        ASSERT_THAT(reader.ReadSignedDoubleWordLE(), Eq(32767));
        ASSERT_THAT(reader.ReadSignedDoubleWordLE(), Eq(-32768));
        ASSERT_THAT(reader.ReadSignedDoubleWordLE(), Eq(2147483647));
        ASSERT_THAT(reader.ReadSignedDoubleWordLE(), Eq(-2147483648));
        ASSERT_THAT(reader.ReadSignedDoubleWordLE(), Eq(1234567890));
        ASSERT_THAT(reader.ReadSignedDoubleWordLE(), Eq(-1234567890));

        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestReadingDWordBE)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa, 0x77, 0xee};

        reader.Initialize(array);
        ASSERT_THAT(reader.ReadDoubleWordBE(), Eq(0x55AA77EEU));
        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestReadingQuadWordLE)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa, 0x77, 0xee, 0x66, 0xcc, 0x44, 0x88};

        reader.Initialize(array);
        ASSERT_THAT(reader.ReadQuadWordLE(), Eq(0x8844CC66EE77AA55ULL));
        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestReadingQuadLEBeyondEnd)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa, 0x77, 0xee, 0x66, 0xcc, 0x44};

        reader.Initialize(array);
        reader.ReadQuadWordLE();
        ASSERT_FALSE(reader.Status());
    }

    TEST(ReaderTest, TestReadingDWordLEBeyondEnd)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa, 0x77};

        reader.Initialize(array);
        reader.ReadDoubleWordLE();
        ASSERT_FALSE(reader.Status());
    }

    TEST(ReaderTest, TestReadArray)
    {
        Reader reader;
        const uint8_t array[] = {0x55, 0xaa, 0x77};

        reader.Initialize(array);
        ASSERT_THAT(reader.ReadArray(3), Eq(gsl::make_span(array)));
        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestReadArrayBeyondEnd)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa, 0x77};

        reader.Initialize(array);
        ASSERT_THAT(reader.ReadArray(4).empty(), Eq(true));
        ASSERT_FALSE(reader.Status());
    }

    TEST(ReaderTest, TestReadingMovesPosition)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa, 0x77, 0xee, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

        reader.Initialize(array);
        ASSERT_THAT(reader.ReadByte(), Eq(0x55));
        ASSERT_THAT(reader.ReadWordLE(), Eq(0x77AA));
        ASSERT_THAT(reader.ReadDoubleWordLE(), Eq(0x332211EEU));
        ASSERT_THAT(reader.ReadArray(3), Eq(gsl::span<const std::uint8_t>(array + 7, 3)));
        ASSERT_THAT(reader.ReadByte(), Eq(0x77));
        ASSERT_THAT(reader.ReadWordBE(), Eq(0x8899));
        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestReadingWithInvalidState)
    {
        Reader reader;
        uint8_t array[] = {0x55};

        reader.Initialize(array);
        reader.ReadByte();
        reader.ReadWordLE();
        reader.ReadWordBE();
        reader.ReadDoubleWordLE();
        reader.ReadArray(3);
        reader.ReadByte();
        ASSERT_FALSE(reader.Status());
    }

    TEST(ReaderTest, TestRemainigSizeNoDataRead)
    {
        Reader reader;
        uint8_t array[] = {0x55};

        reader.Initialize(array);
        ASSERT_THAT(reader.RemainingSize(), Eq(1));
    }

    TEST(ReaderTest, TestRemainigSize)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa, 0x77, 0xee, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

        reader.Initialize(array);
        reader.ReadByte();
        reader.ReadWordLE();
        reader.ReadDoubleWordLE();
        ASSERT_THAT(reader.RemainingSize(), Eq(static_cast<int32_t>(COUNT_OF(array) - 7)));
    }

    TEST(ReaderTest, TestRemainigSizeAtTheEnd)
    {
        Reader reader;
        uint8_t array[] = {0x55, 0xaa, 0x77, 0xee, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

        reader.Initialize(array);
        reader.ReadByte();
        reader.ReadWordLE();
        reader.ReadDoubleWordLE();
        reader.ReadArray(3);
        reader.ReadByte();
        reader.ReadWordBE();
        ASSERT_THAT(reader.RemainingSize(), Eq(0));
    }

    TEST(ReaderTest, TestReadingToEnd)
    {
        std::array<uint8_t, 5> a{1, 2, 3, 4, 5};

        Reader reader(a);

        reader.ReadByte();

        ASSERT_THAT(reader.ReadToEnd(), ElementsAre(2, 3, 4, 5));
    }

    TEST(ReaderTest, TestReadingToEndWhenNothingLeftIsCorrect)
    {
        std::array<uint8_t, 1> a{1};

        Reader reader(a);

        reader.ReadByte();

        ASSERT_THAT(reader.ReadToEnd().size(), Eq(0));
        ASSERT_THAT(reader.Status(), Eq(true));
    }

    TEST(ReaderTest, TestReadingSingleBCDByte)
    {
        Reader reader;
        uint8_t array[] = {0x11, 0xaa};

        reader.Initialize(array);
        ASSERT_THAT(reader.ReadByteBCD(0xf0), Eq(11));
        ASSERT_TRUE(reader.Status());
    }

    TEST(ReaderTest, TestReadingSingleBCDByteWithPartialNibbleMask)
    {
        Reader reader;
        uint8_t array[] = {0b10100010, 0xaa};

        reader.Initialize(array);
        ASSERT_THAT(reader.ReadByteBCD(0b00110000), Eq(22));
        ASSERT_TRUE(reader.Status());
    }
}
