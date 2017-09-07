#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "boot/settings.hpp"
#include "mock/comm.hpp"
#include "mock/fm25w.hpp"
#include "obc/telecommands/boot_settings.hpp"

using boot::BootSettings;
using testing::Eq;
using testing::Invoke;
using testing::_;
using testing::ElementsAre;
using telecommunication::downlink::DownlinkAPID;
using obc::telecommands::SetBootSlotsTelecommand;

class SetBootSlotsTelecommandTest : public testing::Test
{
  protected:
    SetBootSlotsTelecommandTest();
    template <typename... T> void Run(T... params);

    std::array<std::uint8_t, 16> _memory;
    testing::NiceMock<FM25WDriverMock> _fram;
    BootSettings _settings;
    testing::NiceMock<TransmitterMock> _transmitter;

    SetBootSlotsTelecommand _telecommand;
};

SetBootSlotsTelecommandTest::SetBootSlotsTelecommandTest() : _settings(_fram), _telecommand(_settings)
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

template <typename... T> void SetBootSlotsTelecommandTest::Run(T... params)
{
    std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

    _telecommand.Handle(_transmitter, buffer);
}

TEST_F(SetBootSlotsTelecommandTest, ShouldSetBootSlots)
{
    this->_memory[0] = 0xAA;
    EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::BootSlotsInfo, 0x0, ElementsAre(0x11, 0x0, 0x15, 0x31))));

    Run(0x11, 0x15, 0x31);
    ASSERT_THAT(_settings.BootSlots(), Eq(0x15));
    ASSERT_THAT(_settings.FailsafeBootSlots(), Eq(0x31));
}

TEST_F(SetBootSlotsTelecommandTest, ShouldSendErrorFrameForInvalidRequest)
{
    EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::BootSlotsInfo, 0x0, ElementsAre(0x00, 0xE0))));

    Run(0x0, 0x02);
}

TEST_F(SetBootSlotsTelecommandTest, ShouldSendErrorFrameForInvalidBootSlot)
{
    EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::BootSlotsInfo, 0x0, ElementsAre(0x11, 0xE1))));

    _settings.BootSlots(0x15);
    _settings.FailsafeBootSlots(0x13);

    Run(0x11, 0x00, 0x31);

    ASSERT_THAT(_settings.BootSlots(), Eq(0x15));
    ASSERT_THAT(_settings.FailsafeBootSlots(), Eq(0x13));
}

TEST_F(SetBootSlotsTelecommandTest, ShouldSendErrorFrameForInvalidFailsafeBootSlot)
{
    EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::BootSlotsInfo, 0x0, ElementsAre(0x11, 0xE2))));

    _settings.BootSlots(0x15);
    _settings.FailsafeBootSlots(0x13);

    Run(0x11, 0x31, 0x00);

    ASSERT_THAT(_settings.BootSlots(), Eq(0x15));
    ASSERT_THAT(_settings.FailsafeBootSlots(), Eq(0x13));
}
