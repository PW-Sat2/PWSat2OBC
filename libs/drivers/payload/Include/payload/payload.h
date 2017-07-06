#ifndef LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_H_
#define LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_H_

#include <cstdint>
#include <gsl/span>
#include "base/os.h"
#include "error_counter/error_counter.hpp"
#include "gpio/InterruptPinDriver.h"
#include "i2c/i2c.h"
#include "utils.h"

#include "interfaces.h"

namespace drivers
{
    namespace payload
    {
        /**
         * @defgroup payload_hardware Payload Hardware Driver
         * @ingroup payload
         *
         * @brief This module contains hardware driver Payload Devices
         *
         * @{
         */

        /**
         * @brief Payload hardware driver
         */
        class PayloadDriver : public IPayloadDriver
        {
          public:
            /**
             * @brief I2CAddress Address of Payload uC on I2C bus.
             */
            static constexpr std::uint8_t I2CAddress = 0b0110000;

            /**
             * @brief Ctor
             * @param errors Error counting mechanism
             * @param communicationBus I2C bus for communication
             * @param interruptPinDriver Interrupt/busy pin driver
             */
            PayloadDriver(error_counter::ErrorCounting& errors,
                drivers::i2c::II2CBus& communicationBus,
                drivers::gpio::IInterruptPinDriver& interruptPinDriver);

            /**
             * @brief Performs driver initialization
             */
            void Initialize();

            /**
             * @brief Interrupt handler for Payload GPIO pin.
             */
            void IRQHandler();

            /**
             * @brief Returns value indicating if payload is busy and performing measurements.
             * @return True if Payload is busy.
             */
            virtual bool IsBusy() const override;

            /**
              * @brief Executes read from Payload using hardware driver.
              * @param outData Buffer containing parameter - address of requested data.
              * @param inData Buffer for incoming data.
              * @return Result status.
              */
            virtual OSResult PayloadRead(gsl::span<std::uint8_t> outData, gsl::span<std::uint8_t> inData) override;

            /**
             * @brief Executes write to Payload using hardware driver.
             * @param outData Buffer containing parameter - address of measurement data.
             * @return Result status.
             */
            virtual OSResult PayloadWrite(gsl::span<std::uint8_t> outData) override;

            /**
             * @brief Method waiting for Payload be ready to data read.
             * @return Result status.
             */
            virtual OSResult WaitForData() override;

            /**
             * @brief Method executed when data from Payload are ready to read. Should only be called from interrupt service routine.
             * @return Result status.
             */
            virtual OSResult RaiseDataReadyISR() override;

            /**
             * @brief Method setting timeout for data wait.
             * @param newTimeout The new timeout in milliseconds.
             */
            virtual void SetDataTimeout(std::chrono::milliseconds newTimeout) override;

            /** @brief Error counter type */
            using ErrorCounter = error_counter::ErrorCounter<9>;

          private:
            /*
             * @brief DefaultTimeout Default timeout for Payload operations. 5 minutes.
             */
            static constexpr std::chrono::milliseconds DefaultTimeout = std::chrono::milliseconds(5 * 60 * 1000);

            /** @brief Error reporter type */
            using ErrorReporter = error_counter::AggregatedErrorReporter<ErrorCounter::DeviceId>;
            ErrorCounter _error;

            drivers::i2c::II2CBus& _i2c;
            drivers::gpio::IInterruptPinDriver& _interruptPinDriver;

            /** @brief Synchronization */
            OSSemaphoreHandle _sync;

            std::chrono::milliseconds _dataWaitTimeout;
        };

        /* @} */
    }
}

#endif /* LIBS_DRIVERS_PAYLOAD_INCLUDE_PAYLOAD_H_ */
