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
#include "os/os.hpp"

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
using testing::Return;
using testing::AtLeast;

using drivers::spi::ISPIInterface;
using namespace devices::n25q;

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
    EraseChip = 0xC7,
    ClearFlagRegister = 0x50,
    ResetEnable = 0x66,
    ResetMemory = 0x99,
    WriteStatusRegister = 0x01
};

MATCHER_P(CommandCall, command, std::string("Command " + PrintToString(command)))
{
    return arg.size() == 1 && arg[0] == command;
}

static Status operator|(Status lhs, Status rhs)
{
    using U = std::underlying_type_t<Status>;

    return static_cast<Status>(static_cast<U>(lhs) | static_cast<U>(rhs));
}

static FlagStatus operator|(FlagStatus lhs, FlagStatus rhs)
{
    using U = std::underlying_type_t<FlagStatus>;

    return static_cast<FlagStatus>(static_cast<U>(lhs) | static_cast<U>(rhs));
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

    void ExpectCommandAndRespondOnce(Command command, span<uint8_t> response)
    {
        EXPECT_CALL(this->_spi, Write(CommandCall(command)));
        EXPECT_CALL(this->_spi, Read(SpanOfSize(response.size()))).WillOnce(FillBuffer<0>(response));
    }

    void ExpectCommandAndRespondOnce(Command command, uint8_t response)
    {
        EXPECT_CALL(this->_spi, Write(CommandCall(command)));
        EXPECT_CALL(this->_spi, Read(SpanOfSize(1))).WillOnce(FillBuffer<0>(response));
    }

    void ExpectCommandAndRespondOnce(Command command, Status response)
    {
        EXPECT_CALL(this->_spi, Write(CommandCall(command)));
        EXPECT_CALL(this->_spi, Read(SpanOfSize(1))).WillOnce(FillBuffer<0>(num(response)));
    }

    void ExpectCommandAndRespondOnce(Command command, FlagStatus response)
    {
        EXPECT_CALL(this->_spi, Write(CommandCall(command)));
        EXPECT_CALL(this->_spi, Read(SpanOfSize(1))).WillOnce(FillBuffer<0>(num(response)));
    }

    void ExpectCommandAndRespondManyTimes(Command command, uint8_t response, uint16_t times)
    {
        for (auto i = 0; i < times; i++)
        {
            EXPECT_CALL(this->_spi, Write(CommandCall(command)));
            EXPECT_CALL(this->_spi, Read(SpanOfSize(1))).WillOnce(FillBuffer<0>(response));
        }
    }

    void ExpectWaitBusy(uint16_t busyCycles)
    {
        auto selected = this->_spi.ExpectSelected();

        ExpectCommandAndRespondManyTimes(Command::ReadStatusRegister, num(Status::WriteEnabled | Status::WriteInProgress), busyCycles);

        ExpectCommandAndRespondOnce(Command::ReadStatusRegister, Status::WriteDisabled);
    }

    void ExpectClearFlags()
    {
        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::ClearFlagRegister);
        }
    }

    StrictMock<SPIInterfaceMock> _spi;
    N25QDriver _driver;
    NiceMock<OSMock> _os;
    OSReset _osReset;

    std::array<uint8_t, 3> _incorrectId;
    std::array<uint8_t, 3> _correctId;
};

N25QDriverTest::N25QDriverTest() : _driver(_spi), _incorrectId{0xAA, 0xBB, 0xCC}, _correctId{0x20, 0xBA, 0x18}
{
    this->_osReset = InstallProxy(&this->_os);

    ON_CALL(this->_os, GetUptime()).WillByDefault(Return(std::chrono::milliseconds(0)));
}

TEST_F(N25QDriverTest, ShouldReadIdCorrectly)
{
    {
        InSequence s;

        auto selected = this->_spi.ExpectSelected();

        ExpectCommandAndRespondOnce(Command::ReadId, _correctId);
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

        EXPECT_CALL(this->_spi, Write(ElementsAre(Command::ReadMemory, 0xAB, 0x00, 0x00)));

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

        ExpectClearFlags();

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            EXPECT_CALL(this->_spi, Write(ElementsAre(Command::ProgramMemory, 0xAB, 0x00, 0x00)));

            EXPECT_CALL(this->_spi, Write(ContainerEq(span<const uint8_t>(buffer))));
        }

        ExpectWaitBusy(2);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::Clear);
        }
    }

    auto result = this->_driver.WriteMemory(address, buffer);
    ASSERT_THAT(result, Eq(OperationResult::Success));
}

TEST_F(N25QDriverTest, ShouldWriteTwoPages)
{
    const uint32_t address = 0xAB0000;

    array<uint8_t, 356> buffer;
    buffer.fill(0xCC);

    {
        InSequence s;

        ExpectClearFlags();

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            EXPECT_CALL(this->_spi, Write(ElementsAre(Command::ProgramMemory, 0xAB, 0x00, 0x00)));

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

            EXPECT_CALL(this->_spi, Write(ElementsAre(Command::ProgramMemory, 0xAB, 0x01, 0x00)));

            EXPECT_CALL(this->_spi, Write(ContainerEq(span<const uint8_t>(buffer).subspan(256, 100))));
        }

        ExpectWaitBusy(3);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::Clear);
        }
    }

    auto result = this->_driver.WriteMemory(address, buffer);

    ASSERT_THAT(result, Eq(OperationResult::Success));
}

TEST_F(N25QDriverTest, ShouldDetectProgramErrors)
{
    const uint32_t address = 0xAB0000;

    array<uint8_t, 356> buffer;
    buffer.fill(0xCC);

    {
        InSequence s;

        ExpectClearFlags();

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            EXPECT_CALL(this->_spi, Write(ElementsAre(Command::ProgramMemory, 0xAB, 0x00, 0x00)));

            EXPECT_CALL(this->_spi, Write(ContainerEq(span<const uint8_t>(buffer).subspan(0, 256))));
        }

        ExpectWaitBusy(4);

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadFlagStatusRegister, FlagStatus::ProgramError);
        }
    }

    auto result = this->_driver.WriteMemory(address, buffer);

    ASSERT_THAT(result, Eq(OperationResult::Failure));
}

TEST_F(N25QDriverTest, ShouldEraseSubsector)
{
    const uint32_t address = 0xCD0000;

    {
        InSequence s;

        ExpectClearFlags();

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

    ASSERT_THAT(result, Eq(OperationResult::Success));
}

TEST_F(N25QDriverTest, ShouldDetectEraseSubsectorError)
{
    const uint32_t address = 0xCD0000;

    {
        InSequence s;

        ExpectClearFlags();

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

    ASSERT_THAT(result, Eq(OperationResult::Failure));
}

TEST_F(N25QDriverTest, ShouldDetectEraseSubsectorTimeout)
{
    const uint32_t address = 0xCD0000;

    {
        InSequence s;

        ExpectClearFlags();

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::EraseSubsector);

            EXPECT_CALL(this->_spi, Write(ElementsAre(0xCD, 0x00, 0x00)));
        }

        {
            auto selected = this->_spi.ExpectSelected();

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(0)));

            ExpectCommandAndRespondOnce(Command::ReadStatusRegister, Status::WriteEnabled | Status::WriteInProgress);

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(0)));

            ExpectCommandAndRespondOnce(Command::ReadStatusRegister, Status::WriteEnabled | Status::WriteInProgress);

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(std::numeric_limits<uint32_t>::max())));
        }
    }

    auto result = this->_driver.EraseSubSector(address);

    ASSERT_THAT(result, Eq(OperationResult::Timeout));
}

TEST_F(N25QDriverTest, ShouldEraseSector)
{
    const uint32_t address = 0xCD0000;

    {
        InSequence s;

        ExpectClearFlags();

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

    ASSERT_THAT(result, Eq(OperationResult::Success));
}

TEST_F(N25QDriverTest, ShouldDetectEraseSectorError)
{
    const uint32_t address = 0xCD0000;

    {
        InSequence s;

        ExpectClearFlags();

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

    ASSERT_THAT(result, Eq(OperationResult::Failure));
}

TEST_F(N25QDriverTest, ShouldDetectEraseSectorTimeout)
{
    const uint32_t address = 0xCD0000;

    {
        InSequence s;

        ExpectClearFlags();

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::EraseSector);

            EXPECT_CALL(this->_spi, Write(ElementsAre(0xCD, 0x00, 0x00)));
        }

        {
            auto selected = this->_spi.ExpectSelected();

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(0)));

            ExpectCommandAndRespondOnce(Command::ReadStatusRegister, Status::WriteEnabled | Status::WriteInProgress);

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(0)));

            ExpectCommandAndRespondOnce(Command::ReadStatusRegister, Status::WriteEnabled | Status::WriteInProgress);

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(std::numeric_limits<uint32_t>::max())));
        }
    }

    auto result = this->_driver.EraseSector(address);

    ASSERT_THAT(result, Eq(OperationResult::Timeout));
}

TEST_F(N25QDriverTest, ShouldEraseChip)
{
    {
        InSequence s;

        ExpectClearFlags();

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

    ASSERT_THAT(result, Eq(OperationResult::Success));
}

TEST_F(N25QDriverTest, ShouldDetectEraseChipError)
{
    {
        InSequence s;

        ExpectClearFlags();

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

    ASSERT_THAT(result, Eq(OperationResult::Failure));
}

TEST_F(N25QDriverTest, EraseChipOperationWillTimeout)
{
    {
        InSequence s;

        ExpectClearFlags();

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::EraseChip);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(0)));

            ExpectCommandAndRespondOnce(Command::ReadStatusRegister, Status::WriteEnabled | Status::WriteInProgress);

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(0)));

            ExpectCommandAndRespondOnce(Command::ReadStatusRegister, Status::WriteEnabled | Status::WriteInProgress);

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(std::numeric_limits<uint32_t>::max())));
        }
    }

    auto result = this->_driver.EraseChip();

    ASSERT_THAT(result, Eq(OperationResult::Timeout));
}

TEST_F(N25QDriverTest, ClearFlagRegister)
{
    ExpectClearFlags();

    this->_driver.ClearFlags();
}

TEST_F(N25QDriverTest, ShouldResetProperly)
{
    {
        InSequence s;

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::ResetEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::ResetMemory);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadId, _incorrectId);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadId, _correctId);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteStatusRegister);
            EXPECT_CALL(this->_spi, Write(ElementsAre(0x20)));
        }

        ExpectWaitBusy(3);
    }

    auto result = this->_driver.Reset();

    ASSERT_THAT(result, Eq(OperationResult::Success));
}

TEST_F(N25QDriverTest, SettingProtectionOnResetCanTimeout)
{
    {
        InSequence s;

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::ResetEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::ResetMemory);
        }

        EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(0)));

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadId, _correctId);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::WriteStatusRegister);
            EXPECT_CALL(this->_spi, Write(ElementsAre(0x20)));
        }

        {
            auto selected = this->_spi.ExpectSelected();

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(0)));

            ExpectCommandAndRespondOnce(Command::ReadStatusRegister, Status::WriteEnabled | Status::WriteInProgress);

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(0)));

            ExpectCommandAndRespondOnce(Command::ReadStatusRegister, Status::WriteEnabled | Status::WriteInProgress);

            EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(std::numeric_limits<uint32_t>::max())));
        }
    }

    auto result = this->_driver.Reset();

    ASSERT_THAT(result, Eq(OperationResult::Timeout));
}

TEST_F(N25QDriverTest, WaitingOnResetCanTimeout)
{
    {
        InSequence s;

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::ResetEnable);
        }

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommand(Command::ResetMemory);
        }

        EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(0)));
        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadId, _incorrectId);
        }
        EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(0)));

        {
            auto selected = this->_spi.ExpectSelected();

            ExpectCommandAndRespondOnce(Command::ReadId, _incorrectId);
        }
        EXPECT_CALL(this->_os, GetUptime()).WillRepeatedly(Return(std::chrono::milliseconds(std::numeric_limits<uint32_t>::max())));
    }

    auto result = this->_driver.Reset();

    ASSERT_THAT(result, Eq(OperationResult::Timeout));
}
