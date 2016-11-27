#include <algorithm>
#include <array>

#include <gsl/span>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "n25q/n25q.h"
#include "spi/spi.h"
#include "utils.hpp"

using std::array;
using std::copy;
using gsl::span;

using testing::Test;
using testing::ContainerEq;
using testing::Eq;
using testing::NiceMock;
using testing::StrictMock;
using testing::_;
using testing::Invoke;
using testing::ElementsAre;
using testing::PrintToString;
using testing::InSequence;
using testing::WithArg;

using drivers::spi::ISPIInterface;
using namespace devices::n25q;

struct SPIInterfaceMock;

class SPIExpectSelected
{
  public:
    SPIExpectSelected(SPIInterfaceMock& mock);
    ~SPIExpectSelected();

  private:
    SPIInterfaceMock& _mock;
};

struct SPIInterfaceMock : ISPIInterface
{
    MOCK_METHOD0(Select, void());
    MOCK_METHOD0(Deselect, void());
    MOCK_METHOD1(Write, void(gsl::span<const std::uint8_t> buffer));
    MOCK_METHOD1(Read, void(gsl::span<std::uint8_t> buffer));
    MOCK_METHOD2(WriteRead, void(gsl::span<const std::uint8_t> input, gsl::span<std::uint8_t> output));

    SPIExpectSelected ExpectSelected();
};

SPIExpectSelected::SPIExpectSelected(SPIInterfaceMock& mock) : _mock(mock)
{
    EXPECT_CALL(_mock, Select()).Times(1);
}

SPIExpectSelected::~SPIExpectSelected()
{
    EXPECT_CALL(_mock, Deselect()).Times(1);
}

SPIExpectSelected SPIInterfaceMock::ExpectSelected()
{
    return SPIExpectSelected(*this);
}

enum Command
{
    ReadId = 0x9E,
    ReadStatusRegister = 0x05,
    ReadFlagStatusRegister = 0x70,
    ReadMemory = 0x03,
    WriteEnable = 0x06,
    WriteDisable = 0x05,
    ProgramMemory = 0x02,
    EraseSubsector = 0x20,
    EraseSector = 0xD8,
    EraseChip = 0xC7
};

MATCHER_P(SingletonSpan, expected, std::string("Span contains single item " + PrintToString(expected)))
{
    return arg.size() == 1 && arg[0] == expected;
}

MATCHER_P(SpanOfSize, expected, std::string("Span has size " + PrintToString(expected)))
{
    return arg.size() == expected;
}

template <std::size_t Arg> auto FillBuffer(span<uint8_t> items)
{
    return WithArg<Arg>(Invoke([items](span<uint8_t> buffer) { std::copy(items.cbegin(), items.cend(), buffer.begin()); }));
}

template <std::size_t Arg> auto FillBuffer(uint8_t value)
{
    return WithArg<Arg>(Invoke([value](span<uint8_t> buffer) { buffer[0] = value; }));
}

MATCHER_P(CommandCall, command, std::string("Command " + PrintToString(command)))
{
    return arg.size() == 1 && arg[0] == command;
}

class N25QDriverTest : public Test
{
  public:
    N25QDriverTest();

  protected:
    decltype(auto) ExpectCommand(Command command)
    {
        return EXPECT_CALL(this->_spi, Write(CommandCall(command)));
    }

    decltype(auto) ExpectCommandAndRespondOnce(Command command, span<uint8_t> response)
    {
        return EXPECT_CALL(this->_spi, WriteRead(CommandCall(command), SpanOfSize(response.size()))).WillOnce(FillBuffer<1>(response));
    }

    decltype(auto) ExpectCommandAndRespondOnce(Command command, uint8_t response)
    {
        return EXPECT_CALL(this->_spi, WriteRead(CommandCall(command), SpanOfSize(1))).WillOnce(FillBuffer<1>(response));
    }

    decltype(auto) ExpectCommandAndRespondManyTimes(Command command, uint8_t response, uint16_t times)
    {
        return EXPECT_CALL(this->_spi, WriteRead(CommandCall(command), SpanOfSize(1))).Times(times).WillRepeatedly(FillBuffer<1>(response));
    }

    void ExpectWaitBusy(uint16_t busyCycles)
    {
        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondManyTimes(Command::ReadStatusRegister, Status::WriteEnabled | Status::WriteInProgress, busyCycles);

            ExpectCommandAndRespondOnce(Command::ReadStatusRegister, Status::WriteDisabled);
        }
    }

    StrictMock<SPIInterfaceMock> _spi;
    N25QDriver _driver;
};

N25QDriverTest::N25QDriverTest() : _driver(_spi)
{
}

TEST_F(N25QDriverTest, ShouldReadIdCorrectly)
{
    array<uint8_t, 3> deviceId{0x20, 0xBA, 0x18};

    {
        InSequence s;

        auto selected = this->_spi.ExpectSelected();

        ExpectCommandAndRespondOnce(Command::ReadId, deviceId);
    }

    auto id = this->_driver.ReadId();

    ASSERT_THAT(id.Manufacturer, Eq(0x20));
    ASSERT_THAT(id.MemoryType, Eq(0xBA));
    ASSERT_THAT(id.MemoryCapacity, Eq(0x18));
}

TEST_F(N25QDriverTest, ShouldReadStatusRegisterCorrectly)
{
    {
        InSequence s;

        auto selected = this->_spi.ExpectSelected();

        ExpectCommandAndRespondOnce(Command::ReadStatusRegister, Status::WriteDisabled | Status::WriteInProgress);
    }

    auto status = this->_driver.ReadStatus();

    ASSERT_THAT(status, Eq(Status::WriteDisabled | Status::WriteInProgress));
}

TEST_F(N25QDriverTest, ShouldReadFlagStatusRegisterCorrectly)
{
    {
        InSequence s;

        auto selected = this->_spi.ExpectSelected();

        ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::EraseSuspended | FlagStatus::ProgramError);
    }

    auto status = this->_driver.ReadFlagStatus();

    ASSERT_THAT(status, Eq(FlagStatus::EraseSuspended | FlagStatus::ProgramError));
}

TEST_F(N25QDriverTest, ReadRequestShouldBePropertlyFormed)
{
    const uint32_t address = 0xAB0000;

    array<uint8_t, 260> memory;
    memory.fill(0xCC);

    {
        InSequence s;
        auto selected = this->_spi.ExpectSelected();

        ExpectCommand(Command::ReadMemory);

        EXPECT_CALL(this->_spi, Write(ElementsAre(0xAB, 0x00, 0x00)));

        EXPECT_CALL(this->_spi, Read(SpanOfSize(260))).WillOnce(FillBuffer<0>(memory));
    }

    array<uint8_t, 260> buffer;
    buffer.fill(0);

    this->_driver.ReadMemory(address, buffer);

    ASSERT_THAT(buffer, ContainerEq(memory));
}

TEST_F(N25QDriverTest, ShouldWriteSinglePage)
{
    const uint32_t address = 0xAB0000;

    array<uint8_t, 256> buffer;
    buffer.fill(0xCC);

    {
        InSequence s;
        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::ProgramMemory);

            EXPECT_CALL(this->_spi, Write(ElementsAre(0xAB, 0x00, 0x00)));

            EXPECT_CALL(this->_spi, Write(ContainerEq(span<const uint8_t>(buffer))));
        }

        ExpectWaitBusy(2);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::Clear);
        }
    }

    auto result = this->_driver.WriteMemory(address, buffer);
    ASSERT_THAT(result, Eq(true));
}

TEST_F(N25QDriverTest, ShouldWriteTwoPages)
{
    const uint32_t address = 0xAB0000;

    array<uint8_t, 356> buffer;
    buffer.fill(0xCC);

    {
        InSequence s;
        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::ProgramMemory);

            EXPECT_CALL(this->_spi, Write(ElementsAre(0xAB, 0x00, 0x00)));

            EXPECT_CALL(this->_spi, Write(ContainerEq(span<const uint8_t>(buffer).subspan(0, 256))));
        }

        ExpectWaitBusy(4);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::Clear);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::ProgramMemory);

            EXPECT_CALL(this->_spi, Write(ElementsAre(0xAB, 0x01, 0x00)));

            EXPECT_CALL(this->_spi, Write(ContainerEq(span<const uint8_t>(buffer).subspan(256, 100))));
        }

        ExpectWaitBusy(3);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::Clear);
        }
    }

    auto result = this->_driver.WriteMemory(address, buffer);

    ASSERT_THAT(result, Eq(true));
}

TEST_F(N25QDriverTest, ShouldDetectProgramErrors)
{
    const uint32_t address = 0xAB0000;

    array<uint8_t, 356> buffer;
    buffer.fill(0xCC);

    {
        InSequence s;
        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::ProgramMemory);

            EXPECT_CALL(this->_spi, Write(ElementsAre(0xAB, 0x00, 0x00)));

            EXPECT_CALL(this->_spi, Write(ContainerEq(span<const uint8_t>(buffer).subspan(0, 256))));
        }

        ExpectWaitBusy(4);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::ProgramError);
        }
    }

    auto result = this->_driver.WriteMemory(address, buffer);

    ASSERT_THAT(result, Eq(false));
}

TEST_F(N25QDriverTest, ShouldEraseSubsector)
{
    const uint32_t address = 0xCD0000;

    {
        InSequence s;

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::EraseSubsector);

            EXPECT_CALL(this->_spi, Write(ElementsAre(0xCD, 0x00, 0x00)));
        }

        ExpectWaitBusy(4);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::Clear);
        }
    }

    auto result = this->_driver.EraseSubSector(address);

    ASSERT_THAT(result, Eq(true));
}

TEST_F(N25QDriverTest, ShouldDetectEraseSubsectorError)
{
    const uint32_t address = 0xCD0000;

    {
        InSequence s;

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::EraseSubsector);

            EXPECT_CALL(this->_spi, Write(ElementsAre(0xCD, 0x00, 0x00)));
        }

        ExpectWaitBusy(4);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::EraseError);
        }
    }

    auto result = this->_driver.EraseSubSector(address);

    ASSERT_THAT(result, Eq(false));
}

TEST_F(N25QDriverTest, ShouldEraseSector)
{
    const uint32_t address = 0xCD0000;

    {
        InSequence s;

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::EraseSector);

            EXPECT_CALL(this->_spi, Write(ElementsAre(0xCD, 0x00, 0x00)));
        }

        ExpectWaitBusy(4);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::Clear);
        }
    }

    auto result = this->_driver.EraseSector(address);

    ASSERT_THAT(result, Eq(true));
}

TEST_F(N25QDriverTest, ShouldDetectEraseSectorError)
{
    const uint32_t address = 0xCD0000;

    {
        InSequence s;

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::EraseSector);

            EXPECT_CALL(this->_spi, Write(ElementsAre(0xCD, 0x00, 0x00)));
        }

        ExpectWaitBusy(4);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::EraseError);
        }
    }

    auto result = this->_driver.EraseSector(address);

    ASSERT_THAT(result, Eq(false));
}

TEST_F(N25QDriverTest, ShouldEraseChip)
{
    {
        InSequence s;

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::EraseChip);
        }

        ExpectWaitBusy(4);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::Clear);
        }
    }

    auto result = this->_driver.EraseChip();

    ASSERT_THAT(result, Eq(true));
}

TEST_F(N25QDriverTest, ShouldDetectEraseChipError)
{
    {
        InSequence s;

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::EraseChip);
        }

        ExpectWaitBusy(4);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::EraseError);
        }
    }

    auto result = this->_driver.EraseChip();

    ASSERT_THAT(result, Eq(false));
}

TEST_F(N25QDriverTest, ShoulWaitBusyTimeout)
{
    // TODO: write test
}
