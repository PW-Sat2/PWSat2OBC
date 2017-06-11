#include <algorithm>
#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "boot/settings.hpp"
#include "mock/fm25w.hpp"

using testing::NiceMock;
using testing::Eq;
using testing::_;
using testing::Invoke;
using testing::Return;
using testing::Each;
using std::copy;

class BootSettingsTest : public testing::Test
{
  public:
    BootSettingsTest();

  protected:
    void InvalidMagicNumber();

    std::array<std::uint8_t, 1_KB> _memory;
    NiceMock<FM25WDriverMock> _fram;
    boot::BootSettings _settings;
};

void BootSettingsTest::InvalidMagicNumber()
{
    this->_memory[0] = 1;
    this->_memory[1] = 2;
    this->_memory[2] = 3;
    this->_memory[3] = 4;
}

BootSettingsTest::BootSettingsTest() : _settings(_fram)
{
    this->_memory.fill(0xCC);

    this->_memory[0] = 0xD5;
    this->_memory[1] = 0xC5;
    this->_memory[2] = 0x53;
    this->_memory[3] = 0x7D;

    ON_CALL(this->_fram, Write(_, _)).WillByDefault(Invoke([this](devices::fm25w::Address address, gsl::span<const std::uint8_t> buffer) {
        std::copy(buffer.begin(), buffer.end(), this->_memory.begin() + address);
    }));

    ON_CALL(this->_fram, Read(_, _)).WillByDefault(Invoke([this](devices::fm25w::Address address, gsl::span<std::uint8_t> buffer) {
        std::copy(this->_memory.begin() + address, this->_memory.begin() + address + buffer.size(), buffer.begin());
    }));
}

TEST_F(BootSettingsTest, ShouldReturnTrueIfValidMagicNumberFound)
{
    auto result = _settings.CheckMagicNumber();
    ASSERT_THAT(result, Eq(true));
}

TEST_F(BootSettingsTest, ShouldReturnFalseIfValidMagicNumberNotFound)
{
    InvalidMagicNumber();

    auto result = _settings.CheckMagicNumber();
    ASSERT_THAT(result, Eq(false));
}

TEST_F(BootSettingsTest, ShouldWriteValidSignature)
{
    InvalidMagicNumber();

    _settings.MarkAsValid();
    ASSERT_THAT(this->_memory[0], Eq(0xD5));
    ASSERT_THAT(this->_memory[1], Eq(0xC5));
    ASSERT_THAT(this->_memory[2], Eq(0x53));
    ASSERT_THAT(this->_memory[3], Eq(0x7D));
}

TEST_F(BootSettingsTest, ShouldReadBootSlot)
{
    this->_memory[4] = 0xAB;
    auto result = _settings.BootSlots();
    ASSERT_THAT(result, Eq(0xAB));
}

TEST_F(BootSettingsTest, ShouldReturnDefaultBootSlotIfNoValidSignatureFound)
{
    InvalidMagicNumber();
    this->_memory[4] = 0xAB;
    auto result = _settings.BootSlots();
    ASSERT_THAT(result, Eq(0b111));
}

TEST_F(BootSettingsTest, ShouldWriteBootSlot)
{
    auto result = _settings.BootSlots(0b000111);
    ASSERT_THAT(result, Eq(true));
    ASSERT_THAT(this->_memory[4], Eq(0b000111));
}

TEST_F(BootSettingsTest, ShouldReturnErrorWhenSettingBootSlotsReadBackReturnsDifferentValue)
{
    ON_CALL(this->_fram, Write(_, _)).WillByDefault(Return());
    auto result = _settings.BootSlots(0b000111);
    ASSERT_THAT(result, Eq(false));
}

TEST_F(BootSettingsTest, ShouldReadFailsafeBootSlot)
{
    this->_memory[5] = 0xAB;
    auto result = _settings.FailsafeBootSlots();
    ASSERT_THAT(result, Eq(0xAB));
}

TEST_F(BootSettingsTest, ShouldWriteFailsafeBootSlot)
{
    auto result = _settings.FailsafeBootSlots(0b000111);
    ASSERT_THAT(result, Eq(true));
    ASSERT_THAT(this->_memory[5], Eq(0b000111));
}

TEST_F(BootSettingsTest, ShouldReturnDefaultFailsafeBootSlotIfNoValidSignatureFound)
{
    InvalidMagicNumber();
    this->_memory[5] = 0xAB;
    auto result = _settings.FailsafeBootSlots();
    ASSERT_THAT(result, Eq(0b111000));
}

TEST_F(BootSettingsTest, ShouldReturnErrorWhenSettingFailsafeBootSlotsReadBackReturnsDifferentValue)
{
    ON_CALL(this->_fram, Write(_, _)).WillByDefault(Return());
    auto result = _settings.FailsafeBootSlots(0b000111);
    ASSERT_THAT(result, Eq(false));
}

TEST_F(BootSettingsTest, ShouldReadBootCounter)
{
    this->_memory[6] = 0xAB;
    this->_memory[7] = 0xCD;
    this->_memory[8] = 0xEF;
    this->_memory[9] = 0x01;
    auto result = _settings.BootCounter();
    ASSERT_THAT(result, Eq(0x01EFCDABUL));
}

TEST_F(BootSettingsTest, ShouldReadDefaultBootCounterIfNoValidSignatureFound)
{
    InvalidMagicNumber();
    this->_memory[6] = 0xAB;
    this->_memory[7] = 0xCD;
    auto result = _settings.BootCounter();
    ASSERT_THAT(result, Eq(0UL));
}

TEST_F(BootSettingsTest, ShouldWriteBootCounter)
{
    auto result = _settings.BootCounter(0x01EFCDAB);
    ASSERT_THAT(result, Eq(true));
    ASSERT_THAT(this->_memory[6], Eq(0xAB));
    ASSERT_THAT(this->_memory[7], Eq(0xCD));
    ASSERT_THAT(this->_memory[8], Eq(0xEF));
    ASSERT_THAT(this->_memory[9], Eq(0x01));
}

TEST_F(BootSettingsTest, ShouldReturnErrorWhenSettingBootCounterReadBackReturnsDifferentValue)
{
    ON_CALL(this->_fram, Write(_, _)).WillByDefault(Return());
    auto result = _settings.BootCounter(0xABCD);
    ASSERT_THAT(result, Eq(false));
}

TEST_F(BootSettingsTest, ShouldReturnLastConfirmedBootCounter)
{
    this->_memory[10] = 0x12;
    this->_memory[11] = 0x34;
    this->_memory[12] = 0x56;
    this->_memory[13] = 0x78;
    auto result = _settings.LastConfirmedBootCounter();
    ASSERT_THAT(result, Eq(0x78563412UL));
}

TEST_F(BootSettingsTest, ShouldSetLastBootConfirmedBootCounter)
{
    _settings.BootCounter(0x1234);
    _settings.ConfirmBoot();
    ASSERT_THAT(_settings.LastConfirmedBootCounter(), Eq(0x1234UL));
}

TEST_F(BootSettingsTest, TestErase)
{
    _settings.BootSlots(0b111);
    _settings.FailsafeBootSlots(0b111000);
    _settings.MarkAsValid();

    _settings.Erase();

    ASSERT_THAT(gsl::make_span(this->_memory).subspan(0, 9), Each(Eq(0xFF)));
}

TEST_F(BootSettingsTest, ShouldCheckIfBootSlotIsValid)
{
    ASSERT_THAT(boot::BootSettings::IsValidBootSlot(0b10000000), Eq(true));
    ASSERT_THAT(boot::BootSettings::IsValidBootSlot(0b01000000), Eq(true));

    ASSERT_THAT(boot::BootSettings::IsValidBootSlot(0b11000000), Eq(false));
    ASSERT_THAT(boot::BootSettings::IsValidBootSlot(0b00000000), Eq(false));

    ASSERT_THAT(boot::BootSettings::IsValidBootSlot(0b00000001), Eq(false));
    ASSERT_THAT(boot::BootSettings::IsValidBootSlot(0b00000011), Eq(false));
    ASSERT_THAT(boot::BootSettings::IsValidBootSlot(0b00001111), Eq(false));

    ASSERT_THAT(boot::BootSettings::IsValidBootSlot(0b00000111), Eq(true));
    ASSERT_THAT(boot::BootSettings::IsValidBootSlot(0b00111000), Eq(true));
    ASSERT_THAT(boot::BootSettings::IsValidBootSlot(0b00101010), Eq(true));
}
