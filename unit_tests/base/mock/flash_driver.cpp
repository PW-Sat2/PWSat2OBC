#include "flash_driver.hpp"
#include <array>

static std::array<std::uint8_t, 4_MB> Flash;

using program_flash::FlashStatus;
using testing::_;
using testing::Invoke;
using testing::A;
using testing::Return;

FlashDriverMock::FlashDriverMock() : _storage(Flash)
{
    Flash.fill(0xA5);

    ON_CALL(*this, DeviceId()).WillByDefault(Return(0x00530000));
    ON_CALL(*this, BootConfig()).WillByDefault(Return(0x00000002));

    ON_CALL(*this, At(_)).WillByDefault(Invoke([this](std::size_t offset) { return this->_storage.data() + offset; }));

    ON_CALL(*this, EraseSector(_)).WillByDefault(Invoke([this](std::size_t sectorOffset) {
        auto sector = this->_storage.subspan(sectorOffset, 64_KB);
        std::fill(sector.begin(), sector.end(), 0xFF);
        return FlashStatus::NotBusy;
    }));

    ON_CALL(*this, Program(_, A<gsl::span<const std::uint8_t>>()))
        .WillByDefault(Invoke([this](std::size_t offset, gsl::span<const std::uint8_t> value) {
            std::copy(value.begin(), value.end(), this->_storage.begin() + offset);

            return FlashStatus::NotBusy;
        }));
    ON_CALL(*this, Program(_, A<std::uint8_t>())).WillByDefault(Invoke([this](std::size_t offset, std::uint8_t value) {
        this->_storage[offset] = value;
        return FlashStatus::NotBusy;
    }));

    ON_CALL(*this, Lock(_)).WillByDefault(Return(true));
}

FlashDriverMock::~FlashDriverMock()
{
}
