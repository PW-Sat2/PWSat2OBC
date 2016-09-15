#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/reader.h"
#include "system.h"

using testing::Eq;

TEST(ReaderTest, TestStatusNullBuffer)
{
    Reader reader;
    ReaderInitialize(&reader, NULL, 1);
    ASSERT_FALSE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestStatusZeroSizeBuffer)
{
    uint8_t array[1];
    Reader reader;
    ReaderInitialize(&reader, array, 0);
    ASSERT_FALSE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestStatusValidBuffer)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa};

    ReaderInitialize(&reader, array, sizeof(array));
    ASSERT_TRUE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingSingleByte)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa};

    ReaderInitialize(&reader, array, sizeof(array));
    ASSERT_THAT(ReaderReadByte(&reader), Eq(0x55));
    ASSERT_TRUE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingSingleByteBeyondEnd)
{
    Reader reader;
    uint8_t array[] = {0x55};

    ReaderInitialize(&reader, array, sizeof(array));
    ReaderReadByte(&reader);
    ReaderReadByte(&reader);
    ASSERT_FALSE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingWordLE)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa};

    ReaderInitialize(&reader, array, sizeof(array));
    ASSERT_THAT(ReaderReadWordLE(&reader), Eq(0xaa55));
    ASSERT_TRUE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingWordBE)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa};

    ReaderInitialize(&reader, array, sizeof(array));
    ASSERT_THAT(ReaderReadWordBE(&reader), Eq(0x55aa));
    ASSERT_TRUE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingWordLEBeyondEnd)
{
    Reader reader;
    uint8_t array[] = {0x55};

    ReaderInitialize(&reader, array, sizeof(array));
    ReaderReadWordLE(&reader);
    ASSERT_FALSE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingWordBEBeyondEnd)
{
    Reader reader;
    uint8_t array[] = {0x55};

    ReaderInitialize(&reader, array, sizeof(array));
    ReaderReadWordBE(&reader);
    ASSERT_FALSE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingDWordLE)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa, 0x77, 0xee};

    ReaderInitialize(&reader, array, sizeof(array));
    ASSERT_THAT(ReaderReadDoubleWordLE(&reader), Eq(0xEE77AA55U));
    ASSERT_TRUE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingQuadWordLE)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa, 0x77, 0xee, 0x66, 0xcc, 0x44, 0x88};

    ReaderInitialize(&reader, array, sizeof(array));
    ASSERT_THAT(ReaderReadQuadWordLE(&reader), Eq(0x8844CC66EE77AA55ULL));
    ASSERT_TRUE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingQuadLEBeyondEnd)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa, 0x77, 0xee, 0x66, 0xcc, 0x44};

    ReaderInitialize(&reader, array, sizeof(array));
    ReaderReadQuadWordLE(&reader);
    ASSERT_FALSE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingDWordLEBeyondEnd)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa, 0x77};

    ReaderInitialize(&reader, array, sizeof(array));
    ReaderReadDoubleWordLE(&reader);
    ASSERT_FALSE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadArray)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa, 0x77};

    ReaderInitialize(&reader, array, sizeof(array));
    ASSERT_THAT(ReaderReadArray(&reader, 3), Eq(array));
    ASSERT_TRUE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadArrayBeyondEnd)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa, 0x77};

    ReaderInitialize(&reader, array, sizeof(array));
    ASSERT_THAT(ReaderReadArray(&reader, 4), Eq(nullptr));
    ASSERT_FALSE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingMovesPosition)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa, 0x77, 0xee, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

    ReaderInitialize(&reader, array, sizeof(array));
    ASSERT_THAT(ReaderReadByte(&reader), Eq(0x55));
    ASSERT_THAT(ReaderReadWordLE(&reader), Eq(0x77AA));
    ASSERT_THAT(ReaderReadDoubleWordLE(&reader), Eq(0x332211EEU));
    ASSERT_THAT(ReaderReadArray(&reader, 3), Eq(&array[7]));
    ASSERT_THAT(ReaderReadByte(&reader), Eq(0x77));
    ASSERT_THAT(ReaderReadWordBE(&reader), Eq(0x8899));
    ASSERT_TRUE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestReadingWithInvalidState)
{
    Reader reader;
    uint8_t array[] = {0x55};

    ReaderInitialize(&reader, array, sizeof(array));
    ReaderReadByte(&reader);
    ReaderReadWordLE(&reader);
    ReaderReadWordBE(&reader);
    ReaderReadDoubleWordLE(&reader);
    ReaderReadArray(&reader, 3);
    ReaderReadByte(&reader);
    ASSERT_FALSE(ReaderStatus(&reader));
}

TEST(ReaderTest, TestRemainigSizeNoDataRead)
{
    Reader reader;
    uint8_t array[] = {0x55};

    ReaderInitialize(&reader, array, sizeof(array));
    ASSERT_THAT(ReaderRemainingSize(&reader), Eq(1));
}

TEST(ReaderTest, TestRemainigSize)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa, 0x77, 0xee, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};

    ReaderInitialize(&reader, array, sizeof(array));
    ReaderReadByte(&reader);
    ReaderReadWordLE(&reader);
    ReaderReadDoubleWordLE(&reader);
    ASSERT_THAT(ReaderRemainingSize(&reader), Eq(static_cast<int32_t>(COUNT_OF(array) - 7)));
}

TEST(ReaderTest, TestRemainigSizeAtTheEnd)
{
    Reader reader;
    uint8_t array[] = {0x55, 0xaa, 0x77, 0xee, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};

    ReaderInitialize(&reader, array, sizeof(array));
    ReaderReadByte(&reader);
    ReaderReadWordLE(&reader);
    ReaderReadDoubleWordLE(&reader);
    ReaderReadArray(&reader, 3);
    ReaderReadByte(&reader);
    ReaderReadWordBE(&reader);
    ASSERT_THAT(ReaderRemainingSize(&reader), Eq(0));
}
