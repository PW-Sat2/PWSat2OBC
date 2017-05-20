#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "SPI/SPIMock.h"
#include "base/writer.h"
#include "fm25w/fm25w.hpp"
#include "system.h"
#include "utils.hpp"

using testing::StrictMock;
using testing::Eq;
using testing::Return;
using std::uint16_t;
using std::uint8_t;
using testing::_;
using testing::InSequence;
using gsl::span;

using namespace devices::fm25w;
namespace
{
    struct FM25WDriverMock : public IFM25WDriver
    {
        MOCK_METHOD0(ReadStatus, Option<Status>());

        MOCK_METHOD2(Read, void(Address address, gsl::span<std::uint8_t> buffer));

        MOCK_METHOD2(Write, void(Address address, gsl::span<const std::uint8_t> buffer));
    };

    class RedundantFM25WDriverTest : public testing::Test
    {
      protected:
        RedundantFM25WDriverTest();

        StrictMock<FM25WDriverMock> _fm25wDriver[3];
        RedundantFM25WDriver _driver;
    };

    RedundantFM25WDriverTest::RedundantFM25WDriverTest() : _driver{{&_fm25wDriver[0], &_fm25wDriver[1], &_fm25wDriver[2]}}
    {
    }

    TEST_F(RedundantFM25WDriverTest, ShouldReadStatusRegister)
    {
        EXPECT_CALL(_fm25wDriver[0], ReadStatus()).WillOnce(Return(Some(Status::WriteEnabled)));
        EXPECT_CALL(_fm25wDriver[1], ReadStatus()).WillOnce(Return(Some(Status::WriteEnabled)));
        EXPECT_CALL(_fm25wDriver[2], ReadStatus()).WillOnce(Return(Some(Status::WriteEnabled)));

        auto status = _driver.ReadStatus();

        ASSERT_THAT(status, Eq(Some(Status::WriteEnabled)));
    }

    TEST_F(RedundantFM25WDriverTest, ShouldVoteForStatusRegisterValue)
    {
        EXPECT_CALL(_fm25wDriver[0], ReadStatus()).WillOnce(Return(Some(Status::WriteProtect0)));
        EXPECT_CALL(_fm25wDriver[1], ReadStatus()).WillOnce(Return(Some(Status::WriteProtect0)));
        EXPECT_CALL(_fm25wDriver[2], ReadStatus()).WillOnce(Return(Some(Status::WriteProtectEnabled)));

        auto status = _driver.ReadStatus();

        ASSERT_THAT(status, Eq(Some(Status::WriteProtect0)));
    }

    TEST_F(RedundantFM25WDriverTest, ShouldReturnNoneIfAllStatusesAreDifferent)
    {
        EXPECT_CALL(_fm25wDriver[0], ReadStatus()).WillOnce(Return(Some(Status::WriteEnabled)));
        EXPECT_CALL(_fm25wDriver[1], ReadStatus()).WillOnce(Return(Some(Status::WriteProtect0)));
        EXPECT_CALL(_fm25wDriver[2], ReadStatus()).WillOnce(Return(Some(Status::WriteProtectEnabled)));

        auto status = _driver.ReadStatus();

        ASSERT_THAT(status, Eq(None<Status>()));
    }

    TEST_F(RedundantFM25WDriverTest, ShouldWriteToAllDrivers)
    {
        auto address = 1;
        std::array<uint8_t, 256> buffer;
        buffer.fill(0xCC);

        EXPECT_CALL(_fm25wDriver[0], Write(address, span<const uint8_t>(buffer)));
        EXPECT_CALL(_fm25wDriver[1], Write(address, span<const uint8_t>(buffer)));
        EXPECT_CALL(_fm25wDriver[2], Write(address, span<const uint8_t>(buffer)));

        _driver.Write(address, buffer);
    }

    TEST_F(RedundantFM25WDriverTest, ShouldReadTwoFirstDriversIfEverythingsOk)
    {
        auto address = 1;

        std::array<uint8_t, 256> buffer;
        buffer.fill(0x11);

        std::array<uint8_t, 256> chunkValue;
        chunkValue.fill(0xDD);

        std::array<uint8_t, 256> expectedOutputBuffer;
        expectedOutputBuffer.fill(0xDD);

        InSequence s;

        EXPECT_CALL(_fm25wDriver[0], Read(address, SpanOfSize(256))).WillOnce(FillBuffer<1>(chunkValue));
        EXPECT_CALL(_fm25wDriver[1], Read(address, SpanOfSize(256))).WillOnce(FillBuffer<1>(chunkValue));
        EXPECT_CALL(_fm25wDriver[2], Read(_, _)).Times(0);

        _driver.Read(address, buffer);

        ASSERT_THAT(buffer, Eq(expectedOutputBuffer));
    }

    TEST_F(RedundantFM25WDriverTest, ShouldReadThirdDriverIfTwoFirstDiffer)
    {
        auto address = 1;

        std::array<uint8_t, 256> buffer;
        buffer.fill(0x11);

        std::array<uint8_t, 256> chunkValue;
        chunkValue.fill(0xDD);

        std::array<uint8_t, 256> expectedOutputBuffer;
        expectedOutputBuffer.fill(0xDD);

        std::array<uint8_t, 256> invalidChunkValue;
        invalidChunkValue.fill(0xEE);

        InSequence s;

        EXPECT_CALL(_fm25wDriver[0], Read(address, SpanOfSize(256))).WillOnce(FillBuffer<1>(chunkValue));
        EXPECT_CALL(_fm25wDriver[1], Read(address, SpanOfSize(256))).WillOnce(FillBuffer<1>(invalidChunkValue));
        EXPECT_CALL(_fm25wDriver[2], Read(address, SpanOfSize(256))).WillOnce(FillBuffer<1>(chunkValue));

        _driver.Read(address, buffer);

        ASSERT_THAT(buffer, Eq(expectedOutputBuffer));
    }

    TEST_F(RedundantFM25WDriverTest, ShouldReadIn1KBChunks)
    {
        auto address = 1;
        std::array<uint8_t, 2_KB> buffer;
        buffer.fill(0x11);

        std::array<uint8_t, 1_KB> chunkBuffer;
        chunkBuffer.fill(0xCC);

        std::array<uint8_t, 2_KB> expectedOutputBuffer;
        expectedOutputBuffer.fill(0xCC);

        InSequence s;

        EXPECT_CALL(_fm25wDriver[0], Read(address, SpanOfSize(1024))).WillOnce(FillBuffer<1>(chunkBuffer));
        EXPECT_CALL(_fm25wDriver[1], Read(address, SpanOfSize(1024))).WillOnce(FillBuffer<1>(chunkBuffer));

        EXPECT_CALL(_fm25wDriver[0], Read(address + 1024, SpanOfSize(1024))).WillOnce(FillBuffer<1>(chunkBuffer));
        EXPECT_CALL(_fm25wDriver[1], Read(address + 1024, SpanOfSize(1024))).WillOnce(FillBuffer<1>(chunkBuffer));

        EXPECT_CALL(_fm25wDriver[2], Read(_, _)).Times(0);

        _driver.Read(address, buffer);

        ASSERT_THAT(buffer, Eq(expectedOutputBuffer));
    }
}
