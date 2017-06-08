#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_H_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"
#include "gpio/gpio.h"
#include "i2c/i2c.h"
#include "utils.h"

namespace drivers
{
    namespace payload
    {
        /**
         * @defgroup payload Driver for payload
         * @ingroup device_drivers
         *
         * @{
         */

        /**
         * @brief Commands for payload
         */
        enum class PayloadCommands : std::uint8_t
        {
            RefreshSunS = 0x80,
            RefreshTemperatures = 0x81,
            RefreshPhotodiodes = 0x82,
            RefreshRadFET = 0x83,
            RefreshHousekeeping = 0x84
        };

        enum class PayloadDataCodes : std::uint8_t
        {
            WhoAmI = 0x00,
            ActualDataFlag = 0x01,
            BusyDataFlag = 0x02,
            ErrorFlag = 0x03,
            StatusFlag = 0x04,
            SunSRefV1 = 0x05,
            SunSRefV2 = 0x07,
            SunSRefV3 = 0x09,
            SunSRefV4 = 0x0B,
            SunSRefV5 = 0x0D,
            Xp_PHD = 0x0F,
            Xm_PHD = 0x11,
            Yp_PHD = 0x13,
            Ym_PHD = 0x15,
            SupplyTemperature = 0x17,
            XpTemperature = 0x19,
            XmTemperature = 0x1B,
            YpTemperature = 0x1D,
            YmTemperature = 0x1F,
            SADSTemperature = 0x21,
            SailTemperature = 0x23,
            GyroTemperature = 0x25,
            SupplyTemperature2 = 0x27,
            INT_3V3d = 0x29,
            OBC_3V3 = 0x31,
            RadFETTemperature = 0x29,
            RadFET_VTH1 = 0x2D,
            RadFET_VTH2 = 0x31,
            RadFET_VTH3 = 0x35
        };

        /**
         * @brief Payload driver interface
         */
        struct IPayloadDriver
        {
            /**
             * @brief StartsReturns pointer to data at given offset
             * @param offset Offset into flash (0 means first byte of flash)
             * @return Status
             */
            virtual OSResult RefreshRadFET() = 0;
        };

        /**
         * @brief Payload driver
         */
        class PayloadDriver : public IPayloadDriver
        {
          public:
            /*
             * @brief I2CAddress Address of Payload uC on I2C bus.
             */
            static constexpr std::uint8_t I2CAddress = 0b0110000;

            /**
             * @brief Ctor
             * @param communicationBus I2C bus for communication
             * @param interruptPin Interrupt/busy pin
             */
            PayloadDriver(drivers::i2c::II2CBus& communicationBus, const drivers::gpio::Pin& interruptPin);

            /**
             * @brief Performs driver initialization
             */
            void Initialize();

            virtual OSResult RefreshRadFET();

            void IRQHandler();

            inline uint16_t IRQMask()
            {
                return 1 << (_interruptPin.PinNumber() / 2);
            }

          private:
            /*
             * @brief DefaultTimeout Default timeout for Payload operations. 30 minutes.
             */
            static constexpr std::chrono::milliseconds DefaultTimeout = std::chrono::milliseconds(1800000);
            static constexpr uint32_t DefaultTimeout2 = 1800000;
            // TODO: fix that

            drivers::i2c::II2CBus& _i2c;
            const drivers::gpio::Pin& _interruptPin;

            /** @brief Synchronization */
            OSSemaphoreHandle _sync;

            OSResult PerformCommand(PayloadCommands command);
            OSResult PerformDataRead(uint8_t address, gsl::span<uint8_t> buffer);
            OSResult ValidateData(gsl::span<uint8_t> buffer);
        };

        /* @} */
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_H_ */
