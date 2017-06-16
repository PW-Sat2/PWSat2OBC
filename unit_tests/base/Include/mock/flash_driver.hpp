#ifndef UNIT_TESTS_BASE_INCLUDE_MOCK_FLASH_DRIVER_HPP_
#define UNIT_TESTS_BASE_INCLUDE_MOCK_FLASH_DRIVER_HPP_

#include "gmock/gmock.h"
#include "program_flash/flash_driver.hpp"

class FlashDriverMock : public program_flash::IFlashDriver
{
  public:
    FlashDriverMock();
    ~FlashDriverMock();

    MOCK_CONST_METHOD1(At, std::uint8_t const*(std::size_t offset));
    MOCK_METHOD2(Program, program_flash::FlashStatus(std::size_t offset, std::uint8_t value));
    MOCK_METHOD2(Program, program_flash::FlashStatus(std::size_t offset, gsl::span<const std::uint8_t> value));
    MOCK_METHOD1(EraseSector, program_flash::FlashStatus(std::size_t sectorOfffset));

    MOCK_CONST_METHOD0(DeviceId, std::uint32_t());
    MOCK_CONST_METHOD0(BootConfig, std::uint32_t());
    MOCK_METHOD1(Lock, bool(std::chrono::milliseconds timeout));
    MOCK_METHOD0(Unlock, void());

    inline gsl::span<std::uint8_t> Storage();

  protected:
    gsl::span<std::uint8_t> _storage;
};

gsl::span<std::uint8_t> FlashDriverMock::Storage()
{
    return this->_storage;
}

#endif /* UNIT_TESTS_BASE_INCLUDE_MOCK_FLASH_DRIVER_HPP_ */
