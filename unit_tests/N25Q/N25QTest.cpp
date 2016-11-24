#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "n25q/n25q.h"
#include "spi/spi.h"
#include "utils.hpp"

using testing::Test;
using testing::Eq;
using testing::NiceMock;
using testing::_;
using testing::Invoke;
using testing::ElementsAre;
using testing::PrintToString;
using testing::InSequence;
using drivers::spi::ISPIInterface;
using namespace devices::n25q;

struct SPIInterfaceMock : ISPIInterface
{
    MOCK_METHOD0(Select, void());
    MOCK_METHOD0(Deselect, void());
    MOCK_METHOD1(Write, void(gsl::span<const std::uint8_t> buffer));
    MOCK_METHOD1(Read, void(gsl::span<std::uint8_t> buffer));
    MOCK_METHOD2(WriteRead, void(gsl::span<const std::uint8_t> input, gsl::span<std::uint8_t> output));
};

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
    EraseChip = 0xC7
};

class N25QDriverTest : public Test
{
  public:
    N25QDriverTest();

  protected:
    NiceMock<SPIInterfaceMock> _spi;
    N25QDriver _driver;
};

N25QDriverTest::N25QDriverTest() : _driver(_spi)
{
}

MATCHER_P(SingletonSpan, expected, std::string("Span contains single item " + PrintToString(expected)))
{
    return arg.size() == 1 && arg[0] == expected;
}

MATCHER_P(SpanOfSize, expected, std::string("Span has size " + PrintToString(expected)))
{
    return arg.size() == expected;
}

TEST_F(N25QDriverTest, ShouldReadIdCorrectly)
{
    {
        InSequence s;

        EXPECT_CALL(this->_spi, Select()).Times(1);

        EXPECT_CALL(this->_spi, WriteRead(SingletonSpan(Command::ReadId), SpanOfSize(3))) //
            .WillOnce(Invoke([](auto /*input*/, auto output) {
                output[0] = 0x20;
                output[1] = 0xBA;
                output[2] = 0x18;
            }));

        EXPECT_CALL(this->_spi, Deselect()).Times(1);
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

        EXPECT_CALL(this->_spi, Select()).Times(1);

        EXPECT_CALL(this->_spi, WriteRead(SingletonSpan(Command::ReadStatusRegister), SpanOfSize(1))) //
            .WillOnce(Invoke([](auto /*input*/, auto output) { output[0] = Status::WriteDisabled | Status::WriteInProgress; }));

        EXPECT_CALL(this->_spi, Deselect()).Times(1);
    }

    auto status = this->_driver.ReadStatus();

    ASSERT_THAT(status, Eq(Status::WriteDisabled | Status::WriteInProgress));
}

TEST_F(N25QDriverTest, ShouldReadFlagStatusRegisterCorrectly)
{
    {
        InSequence s;

        EXPECT_CALL(this->_spi, Select()).Times(1);

        EXPECT_CALL(this->_spi, WriteRead(SingletonSpan(Command::ReadFlagStatusRegister), SpanOfSize(1))) //
            .WillOnce(Invoke([](auto /*input*/, auto output) { output[0] = FlagStatus::EraseSuspended | FlagStatus::ProgramError; }));

        EXPECT_CALL(this->_spi, Deselect()).Times(1);
    }

    auto status = this->_driver.ReadFlagStatus();

    ASSERT_THAT(status, Eq(FlagStatus::EraseSuspended | FlagStatus::ProgramError));
}
