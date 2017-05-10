#include <algorithm>
#include <array>
#include <cstdint>
#include <gsl/span>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "program_flash/boot_table.hpp"
#include "utils.h"

using testing::_;
using testing::A;
using testing::Invoke;
using testing::Eq;
using testing::Return;
using testing::StrEq;
using testing::Each;

struct FlashDriverMock : public program_flash::IFlashDriver
{
    MOCK_CONST_METHOD1(At, std::uint8_t const*(std::size_t offset));
    MOCK_METHOD2(Program, void(std::size_t offset, std::uint8_t value));
    MOCK_METHOD2(Program, void(std::size_t offset, gsl::span<const std::uint8_t> value));
    MOCK_METHOD1(EraseSector, void(std::size_t sectorOfffset));

    MOCK_CONST_METHOD0(DeviceId, std::uint32_t());
    MOCK_CONST_METHOD0(BootConfig, std::uint32_t());
};

static std::array<std::uint8_t, 4_MB> Flash;

class UploadProgramTest : public testing::Test
{
  public:
    UploadProgramTest();

  protected:
    gsl::span<std::uint8_t> _flash;
    testing::NiceMock<FlashDriverMock> _flashMock;
    program_flash::BootTable _bootTable;
};

UploadProgramTest::UploadProgramTest() : _flash(Flash), _bootTable(_flashMock)
{
    Flash.fill(0xA5);

    ON_CALL(this->_flashMock, DeviceId()).WillByDefault(Return(0x00530000));
    ON_CALL(this->_flashMock, BootConfig()).WillByDefault(Return(0x00000002));

    ON_CALL(this->_flashMock, At(_)).WillByDefault(Invoke([this](std::size_t offset) { return this->_flash.data() + offset; }));

    ON_CALL(this->_flashMock, EraseSector(_)).WillByDefault(Invoke([this](std::size_t sectorOffset) {
        auto sector = this->_flash.subspan(sectorOffset, 512_KB);
        std::fill(sector.begin(), sector.end(), 0xFF);
    }));

    ON_CALL(this->_flashMock, Program(_, A<gsl::span<const std::uint8_t>>()))
        .WillByDefault(Invoke([this](std::size_t offset, gsl::span<const std::uint8_t> value) {
            std::copy(value.begin(), value.end(), this->_flash.begin() + offset);
        }));
    ON_CALL(this->_flashMock, Program(_, A<std::uint8_t>())).WillByDefault(Invoke([this](std::size_t offset, std::uint8_t value) {
        this->_flash[offset] = value;
    }));

    this->_bootTable.Initialize();
}

TEST_F(UploadProgramTest, ReadBootIndex)
{
    this->_flash[0] = 1;

    auto bootIndex = this->_bootTable.BootIndex();

    ASSERT_THAT(bootIndex, Eq(1));
}

TEST_F(UploadProgramTest, ReadBootCounter)
{
    this->_flash[0x00002000] = 16;

    auto bootCounter = this->_bootTable.BootCounter();

    ASSERT_THAT(bootCounter, Eq(16));
}

TEST_F(UploadProgramTest, ReadProgramDetails)
{
    this->_flash[0x00080000] = 0x12;
    this->_flash[0x00080001] = 0x13;
    this->_flash[0x00080002] = 0;
    this->_flash[0x00080003] = 0;

    this->_flash[0x00080020] = 0xAB;
    this->_flash[0x00080021] = 0xCD;

    this->_flash[0x00080040] = 0xAA;

    strcpy(reinterpret_cast<char*>(&this->_flash[0x00080080]), "Test");
    strcpy(reinterpret_cast<char*>(&this->_flash[0x00080400]), "Program");

    auto entry = this->_bootTable.Entry(1);

    ASSERT_THAT(entry.Length(), Eq(0x1312U));
    ASSERT_THAT(entry.Crc(), Eq(0xCDAB));
    ASSERT_THAT(entry.IsValid(), Eq(true));
    ASSERT_THAT(entry.Description(), StrEq("Test"));
    ASSERT_THAT(reinterpret_cast<const char*>(entry.Content()), StrEq("Program"));
}

TEST_F(UploadProgramTest, ProgramSequence)
{
    auto entry = this->_bootTable.Entry(1);

    entry.Erase();

    entry.Length(0x1312U);
    entry.Crc(0xCDAB);
    entry.Description("Test");
    entry.WriteContent(0, gsl::make_span(reinterpret_cast<const uint8_t*>("Program"), 8));
    entry.MarkAsValid();

    ASSERT_THAT(entry.Length(), Eq(0x1312U));
    ASSERT_THAT(entry.Crc(), Eq(0xCDAB));
    ASSERT_THAT(entry.IsValid(), Eq(true));
    ASSERT_THAT(entry.Description(), StrEq("Test"));
    ASSERT_THAT(reinterpret_cast<const char*>(entry.Content()), StrEq("Program"));
}
