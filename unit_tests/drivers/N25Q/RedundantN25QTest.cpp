#include <algorithm>
#include <array>
#include <limits>

#include <gsl/span>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "n25q/n25q.h"
#include "spi/spi.h"
#include "utils.hpp"

#include "OsMock.hpp"
#include "SPI/SPIMock.h"
#include "base/os.h"
#include "mock/error_counter.hpp"
#include "os/os.hpp"

using std::array;
using std::copy;
using gsl::span;

using testing::Test;
using testing::ContainerEq;
using testing::Eq;
using testing::Gt;
using testing::NiceMock;
using testing::StrictMock;
using testing::_;
using testing::Invoke;
using testing::ElementsAre;
using testing::PrintToString;
using testing::InSequence;
using testing::WithArg;
using testing::Return;
using testing::ByMove;
using testing::ReturnRef;
using testing::AtLeast;

using drivers::spi::ISPIInterface;
using namespace devices::n25q;
using namespace std::chrono_literals;

struct N25QDriverMock : public IN25QDriver
{
    MOCK_METHOD2(ReadMemory, OSResult(std::size_t address, gsl::span<uint8_t> buffer));

    MOCK_METHOD3(BeginWritePage, OperationWaiter(size_t address, ptrdiff_t offset, gsl::span<const uint8_t> page));

    MOCK_METHOD1(BeginEraseSubSector, OperationWaiter(size_t address));

    MOCK_METHOD1(BeginEraseSector, OperationWaiter(size_t address));

    MOCK_METHOD0(BeginEraseChip, OperationWaiter());

    MOCK_METHOD0(Reset, OperationResult());

    MOCK_METHOD2(WaitForOperation, OperationResult(std::chrono::milliseconds timeout, FlagStatus status));
};

class RedundantN25QDriverTest : public Test
{
  public:
    RedundantN25QDriverTest()
        : _errors{_errorsConfig},                                               //
          _error_counter{_errors},                                              //
          _driver{_errors, {&_n25qDriver[0], &_n25qDriver[1], &_n25qDriver[2]}} //
    {
    }

    OperationWaiter MakeWaiter(IN25QDriver* driver)
    {
        OperationWaiter op(driver, 1ms, FlagStatus::EraseError);
        return op;
    }

    void ExpectAllWaiters()
    {
        EXPECT_CALL(_n25qDriver[0], WaitForOperation(1ms, FlagStatus::EraseError));
        EXPECT_CALL(_n25qDriver[1], WaitForOperation(1ms, FlagStatus::EraseError));
        EXPECT_CALL(_n25qDriver[2], WaitForOperation(1ms, FlagStatus::EraseError));
    }

    testing::NiceMock<ErrorCountingConfigrationMock> _errorsConfig;
    error_counter::ErrorCounting _errors;
    error_counter::ErrorCounter<RedundantN25QDriver::ErrorCounter::DeviceId> _error_counter;

    StrictMock<N25QDriverMock> _n25qDriver[3];
    RedundantN25QDriver _driver;
};

TEST_F(RedundantN25QDriverTest, ShouldEraseChip)
{
    InSequence s;

    EXPECT_CALL(_n25qDriver[0], BeginEraseChip()).WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[0]))));
    EXPECT_CALL(_n25qDriver[1], BeginEraseChip()).WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[1]))));
    EXPECT_CALL(_n25qDriver[2], BeginEraseChip()).WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[2]))));

    ExpectAllWaiters();

    _driver.EraseChip();

    ASSERT_THAT(_error_counter, Eq(0));
}

TEST_F(RedundantN25QDriverTest, ShouldEraseSector)
{
    InSequence s;

    size_t sectorAddress = 0x0F;

    EXPECT_CALL(_n25qDriver[0], BeginEraseSector(sectorAddress)).WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[0]))));
    EXPECT_CALL(_n25qDriver[1], BeginEraseSector(sectorAddress)).WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[1]))));
    EXPECT_CALL(_n25qDriver[2], BeginEraseSector(sectorAddress)).WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[2]))));

    ExpectAllWaiters();

    _driver.EraseSector(sectorAddress);

    ASSERT_THAT(_error_counter, Eq(0));
}

TEST_F(RedundantN25QDriverTest, ShouldEraseSubSector)
{
    InSequence s;

    size_t sectorAddress = 0x0F;

    EXPECT_CALL(_n25qDriver[0], BeginEraseSubSector(sectorAddress)).WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[0]))));
    EXPECT_CALL(_n25qDriver[1], BeginEraseSubSector(sectorAddress)).WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[1]))));
    EXPECT_CALL(_n25qDriver[2], BeginEraseSubSector(sectorAddress)).WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[2]))));

    ExpectAllWaiters();

    _driver.EraseSubSector(sectorAddress);
}

TEST_F(RedundantN25QDriverTest, ShouldReset)
{
    InSequence s;

    EXPECT_CALL(_n25qDriver[0], Reset()).WillOnce(Return(OperationResult::Success));
    EXPECT_CALL(_n25qDriver[1], Reset()).WillOnce(Return(OperationResult::Success));
    EXPECT_CALL(_n25qDriver[2], Reset()).WillOnce(Return(OperationResult::Success));

    _driver.Reset();

    ASSERT_THAT(_error_counter, Eq(0));
}

TEST_F(RedundantN25QDriverTest, ShouldWriteSinglePageMemory)
{
    array<uint8_t, 256> buffer;
    buffer.fill(0xCC);

    size_t address = 0x0F;

    InSequence s;

    EXPECT_CALL(_n25qDriver[0], BeginWritePage(address, 0, span<const uint8_t>(buffer)))
        .WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[0]))));
    EXPECT_CALL(_n25qDriver[1], BeginWritePage(address, 0, span<const uint8_t>(buffer)))
        .WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[1]))));
    EXPECT_CALL(_n25qDriver[2], BeginWritePage(address, 0, span<const uint8_t>(buffer)))
        .WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[2]))));

    ExpectAllWaiters();

    _driver.WriteMemory(address, buffer);

    ASSERT_THAT(_error_counter, Eq(0));
}

TEST_F(RedundantN25QDriverTest, ShouldReadConsistentMemory)
{
    array<uint8_t, 256> buffer1;
    array<uint8_t, 256> buffer2;
    array<uint8_t, 256> buffer3;
    buffer1.fill(0xCC);
    buffer2.fill(0xCC);

    size_t address = 0x0F;

    InSequence s;

    EXPECT_CALL(_n25qDriver[0], ReadMemory(address, span<uint8_t>(buffer1))).Times(1);
    EXPECT_CALL(_n25qDriver[1], ReadMemory(address, span<uint8_t>(buffer2))).Times(1);
    EXPECT_CALL(_n25qDriver[2], ReadMemory(_, _)).Times(0);

    _driver.ReadMemory(address, buffer1, buffer2, buffer3);

    ASSERT_THAT(_error_counter, Eq(0));
}

TEST_F(RedundantN25QDriverTest, ShouldReadAllChipsIfErrorDetectedMemory)
{
    array<uint8_t, 256> buffer1;
    array<uint8_t, 256> buffer2;
    array<uint8_t, 256> buffer3;
    buffer1.fill(0xCC);
    buffer2.fill(0xCD);
    buffer3.fill(0xCD);

    size_t address = 0x0F;

    InSequence s;

    EXPECT_CALL(_n25qDriver[0], ReadMemory(address, span<uint8_t>(buffer1))).Times(1);
    EXPECT_CALL(_n25qDriver[1], ReadMemory(address, span<uint8_t>(buffer2))).Times(1);
    EXPECT_CALL(_n25qDriver[2], ReadMemory(address, span<uint8_t>(buffer3))).Times(1);

    _driver.ReadMemory(address, buffer1, buffer2, buffer3);

    ASSERT_THAT(buffer1, Eq(buffer2));

    ASSERT_THAT(_error_counter, Eq(5));
}

TEST_F(RedundantN25QDriverTest, ShouldReadShortestLengthOfBuffer)
{
    array<uint8_t, 128> buffer1;
    array<uint8_t, 256> buffer2;
    array<uint8_t, 256> buffer3;
    buffer1.fill(0xCC);
    buffer2.fill(0xCD);
    buffer3.fill(0xCD);

    size_t address = 0x0F;

    InSequence s;

    EXPECT_CALL(_n25qDriver[0], ReadMemory(address, span<uint8_t>(buffer1))).Times(1);
    EXPECT_CALL(_n25qDriver[1], ReadMemory(address, span<uint8_t>(buffer2).subspan(0, 128))).Times(1);
    EXPECT_CALL(_n25qDriver[2], ReadMemory(address, span<uint8_t>(buffer3).subspan(0, 128))).Times(1);

    _driver.ReadMemory(address, buffer1, buffer2, buffer3);

    ASSERT_THAT(span<uint8_t>(buffer1), Eq(span<uint8_t>(buffer2).subspan(0, 128)));

    ASSERT_THAT(_error_counter, Eq(5));
}

TEST_F(RedundantN25QDriverTest, ShouldWriteMultiPageMemory)
{
    constexpr size_t pageSize = 256;
    constexpr uint8_t pageCount = 3;

    array<uint8_t, pageSize * pageCount> buffer;
    buffer.fill(0xCC);

    size_t address = 0x0F;

    InSequence s;

    for (uint8_t i = 0; i < pageCount; ++i)
    {
        auto pageBuffer = span<const uint8_t>(buffer).subspan(i * pageSize, pageSize);

        EXPECT_CALL(_n25qDriver[0], BeginWritePage(address, i * pageSize, pageBuffer))
            .WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[0]))));
        EXPECT_CALL(_n25qDriver[1], BeginWritePage(address, i * pageSize, pageBuffer))
            .WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[1]))));
        EXPECT_CALL(_n25qDriver[2], BeginWritePage(address, i * pageSize, pageBuffer))
            .WillOnce(Return(ByMove(MakeWaiter(&_n25qDriver[2]))));

        ExpectAllWaiters();
    }

    _driver.WriteMemory(address, buffer);

    ASSERT_THAT(_error_counter, Eq(0));
}

TEST_F(RedundantN25QDriverTest, ShouldPropagateReadMemoryTimeout)
{
    array<uint8_t, 256> buffer1;
    array<uint8_t, 256> buffer2;
    array<uint8_t, 256> buffer3;
    buffer1.fill(0xCC);
    buffer2.fill(0xCC);

    size_t address = 0x0F;

    InSequence s;

    EXPECT_CALL(_n25qDriver[0], ReadMemory(address, span<uint8_t>(buffer1))).WillOnce(Return(OSResult::Timeout));
    EXPECT_CALL(_n25qDriver[1], ReadMemory(address, span<uint8_t>(buffer2))).Times(0);
    EXPECT_CALL(_n25qDriver[2], ReadMemory(_, _)).Times(0);

    auto r = _driver.ReadMemory(address, buffer1, buffer2, buffer3);
    ASSERT_THAT(r, Eq(OSResult::Timeout));
}
