#ifndef LIBS_DRIVERS_S29JL_INCLUDE_FLASH_S29JL_HPP_
#define LIBS_DRIVERS_S29JL_INCLUDE_FLASH_S29JL_HPP_

#include "base/os.h"
#include "program_flash/flash_driver.hpp"

namespace devices
{
    namespace s29jl
    {
        /**
             * @brief Flash driver for S29JL
             */
        class FlashDriver : public program_flash::IFlashDriver
        {
          public:
            /**
             * @brief Ctor
             * @param flashBase Pointer for memory area mapped to flash device (EBI)
             */
            explicit FlashDriver(std::uint8_t* flashBase);

            /**
             * @brief Performs driver initialization
             */
            void Initialize();

            virtual inline std::uint32_t DeviceId() const override
            {
                return this->_deviceId;
            }

            virtual inline std::uint32_t BootConfig() const override
            {
                return this->_bootConfig;
            }

            virtual inline std::uint8_t const* At(std::size_t offset) const override
            {
                return this->_flashBase + offset;
            }

            virtual program_flash::FlashStatus EraseSector(std::size_t sectorOfffset) override;

            virtual program_flash::FlashStatus Program(std::size_t offset, std::uint8_t value) override;
            virtual program_flash::FlashStatus Program(std::size_t offset, gsl::span<const std::uint8_t> value) override;

            virtual bool Lock(std::chrono::milliseconds timeout) override;
            virtual void Unlock() override;

          private:
            /**
             * @brief Waits for device to become ready
             * @param offset Offset used to select bank which needs to become ready
             * @return true if device is idle
             */
            bool WaitForIdle(std::size_t offset);

            /** @brief Synchronization */
            OSSemaphoreHandle _sync;
            /** @brief Pointer for memory area mapped to flash device */
            std::uint8_t* _flashBase;
            /** @brief Device ID */
            std::uint32_t _deviceId;
            /** @brief Boot config */
            std::uint8_t _bootConfig;
        };
    }
}

#endif /* LIBS_DRIVERS_S29JL_INCLUDE_FLASH_S29JL_HPP_ */
