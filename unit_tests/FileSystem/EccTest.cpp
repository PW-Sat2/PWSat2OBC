#include "gtest/gtest.h"
#include "gmock/gmock-matchers.h"
#include "base/ecc.h"
#include "system.h"

#include <stdint.h>

using testing::Eq;

TEST(EccTest, ShouldCorrectSingleBitError)
{
    uint8_t data[32];

    for (uint32_t i = 0; i < COUNT_OF(data); i++)
    {
        data[i] = (i * 3) % 256;
    }

    uint32_t read = EccCalc(data, COUNT_OF(data));

    for (uint32_t i = 0; i < COUNT_OF(data) * 8; i++)
    {
        data[i / 8] ^= 1 << (i % 8);

        uint32_t generated = EccCalc(data, COUNT_OF(data));

        EccResult result = EccCorrect(generated, read, data, COUNT_OF(data));

        ASSERT_THAT(result, Eq(EccResultCorrected));
        for (uint32_t j = 0; j < COUNT_OF(data); j++)
        {
            ASSERT_THAT(data[j], Eq((j * 3) % 256));
        }
    }
}

TEST(EccTest, ShouldDetectTwoErrors)
{
    uint8_t data[32];

    for (uint32_t i = 0; i < COUNT_OF(data); i++)
    {
        data[i] = (i * 3) % 256;
    }

    uint32_t read = EccCalc(data, COUNT_OF(data));

    data[23] ^= 0b11;

    uint32_t generated = EccCalc(data, COUNT_OF(data));

    EccResult result = EccCorrect(generated, read, data, COUNT_OF(data));

    ASSERT_THAT(result, Eq(EccResultNotCorrected));
}

TEST(EccTest, ShouldDetectErrorInEcc)
{
    uint8_t data[32];

    for (uint32_t i = 0; i < COUNT_OF(data); i++)
    {
        data[i] = (i * 3) % 256;
    }

    uint32_t read = EccCalc(data, COUNT_OF(data));

    uint32_t generated = read ^ 0b001;

    EccResult result = EccCorrect(generated, read, data, COUNT_OF(data));

    ASSERT_THAT(result, Eq(EccResultCorrupted));
}

TEST(EccTest, ShouldNotDetectAnyErrorsForUnchangedDataAndCode)
{
    uint8_t data[32];

    for (uint32_t i = 0; i < COUNT_OF(data); i++)
    {
        data[i] = (i * 3) % 256;
    }

    uint32_t read = EccCalc(data, COUNT_OF(data));

    uint32_t generated = read;

    EccResult result = EccCorrect(generated, read, data, COUNT_OF(data));

    ASSERT_THAT(result, Eq(EccResultNoError));
}
