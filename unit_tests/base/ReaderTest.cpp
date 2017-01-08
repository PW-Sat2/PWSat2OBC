#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/reader.h"
#include "system.h"

using testing::Eq;
using testing::ElementsAre;

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

TEST(ReaderTest, TestGettingRemainingSpan)
{
    std::array<uint8_t, 5> a{1, 2, 3, 4, 5};

    Reader reader(a);

    reader.ReadByte();

    ASSERT_THAT(reader.Remaining(), ElementsAre(2, 3, 4, 5));
}
