#ifndef SRC_DEVICES_SUNS_HPP_
#define SRC_DEVICES_SUNS_HPP_

#include <array>
#include <chrono>
#include <cstdint>
#include "error_counter/error_counter.hpp"
#include "gpio/InterruptPinDriver.h"
#include "i2c/i2c.h"

/**
 * @defgroup SunSDriver Sun sensor low level driver
 * @ingroup device_drivers
 *
 * @brief This module is a low level sun sensor module driver.
 *
 * This driver is responsible for
 *  - direct communication with the hardware,
 *  - data parsing and interpretation
 *
 *  @{
 */
namespace devices
{
    namespace suns
    {
        /**
         * @brief I2C address of suns board. 7-bit notation.
         */
        constexpr std::uint8_t I2CAddress = 0x44;

        /**
         * @brief Sun sensor operation status codes.
         */
        enum class OperationStatus
        {
            /**
             * @brief No error
             */
            OK,

            /**
             * @brief I2C Write operation failed during command execution
             */
            I2CWriteFailed,

            /**
             * @brief I2C Read operation failed during command execution
             */
            I2CReadFailed,

            /**
             * @brief SunS returned wrong Who-Am-I register value in response
             */
            WhoAmIMismatch,

            /**
             * @brief Synchronization between operation and interrupt has failed
             */
            SynchronizationFailed
        };

        /**
         * @brief Array containing four ALS values.
         */
        using Als = std::array<std::uint16_t, 4>;

        /**
         * @brief Array containing four panel temperatures.
         */
        using Panels = std::array<std::uint16_t, 4>;

        /**
         * @brief Array containing four panel light readings.
         */
        using LightData = std::array<Als, 3>;

        /**
         * @brief Status registers.
         */
        struct Status
        {
            /** @brief ALS ACK. */
            uint16_t ack;

            /** @brief ALS presence. */
            uint16_t presence;

            /** @brief ALS adc_valid. */
            uint16_t adc_valid;
        };

        /**
         * @brief Parameter registers.
         */
        struct Params
        {
            /** @brief Actual gain. */
            uint8_t gain;

            /** @brief Actual itime. */
            uint8_t itime;
        };

        /**
         * @brief Temperature registers.
         */
        struct Temperatures
        {
            /** @brief Structure temperature. */
            std::uint16_t structure;

            /** @brief Panel temperatures. */
            Panels panels;
        };

        /**
         * @brief Structure with Sun sensor registers.
         */
        struct MeasurementData
        {
            /** @brief Status registers. */
            Status status;

            /** @brief Parameter registers. */
            Params parameters;

            /** @brief Temperature registers. */
            Temperatures temperature;

            /** @brief Visible light registers. */
            LightData visible_light;

            /** @brief Infrared registers. */
            LightData infrared;
        };

        /**
         * @brief Interface for sun sensor commanding.
         */
        struct ISunSDriver
        {
            /**
             * Measures Sun Sensor readings.
             * @param[out] data Measurement data
             * @param[in] gain Gain.
             * @param[in] itime Light sensors parameter.
             * @return Operation status.
             */
            virtual OperationStatus MeasureSunS(MeasurementData& data, uint8_t gain, uint8_t itime) = 0;

            /**
             * Starts Sun Sensor measurement.
             * @param[in] gain Gain.
             * @param[in] itime Light sensors parameter.
             * @return Operation status.
             */
            virtual OperationStatus StartMeasurement(uint8_t gain, uint8_t itime) = 0;

            /**
             * Gets Sun Sensor measurement data.
             * @param[out] data Measurement data
             * @return Operation status.
             */
            virtual OperationStatus GetMeasuredData(MeasurementData& data) = 0;

            /**
             * @brief Returns value indicating whether Sun Sensor is busy and performs measurements.
             * @return True when Sun Sensor is busy.
             */
            virtual bool IsBusy() const = 0;

            /**
             * @brief Method waiting for data to be ready to read.
             * @return Result status.
             */
            virtual OSResult WaitForData() = 0;

            /**
             * @brief Method executed when data are ready to read. Should only be called from interrupt service routine.
             */
            virtual void RaiseDataReadyISR() = 0;

            /**
             * @brief Method setting timeout for data wait.
             * @param newTimeout The new timeout in milliseconds.
             */
            virtual void SetDataTimeout(std::chrono::milliseconds newTimeout) = 0;
        };

        /**
         * @brief Low level driver for Sun Sensor commanding.
         */
        class SunSDriver final : public ISunSDriver
        {
          public:
            /** @brief Error counter type. */
            using ErrorCounter = error_counter::ErrorCounter<11>;

            /**
             * Constructs new instance of Sun Sensor low-level driver.
             * @param[in] errors Error counter.
             * @param[in] i2cbus I2C bus used to communicate with device.
             * @param[in] interruptPinDriver Interrupt/busy pin driver.
             */
            SunSDriver(error_counter::ErrorCounting& errors,
                drivers::i2c::II2CBus& i2cbus,
                drivers::gpio::IInterruptPinDriver& interruptPinDriver);

            virtual OperationStatus MeasureSunS(MeasurementData& data, uint8_t gain, uint8_t itime) override;

            virtual OperationStatus StartMeasurement(uint8_t gain, uint8_t itime) override;

            virtual OperationStatus GetMeasuredData(MeasurementData& data) override;

            virtual bool IsBusy() const override;

            virtual OSResult WaitForData() override;

            virtual void RaiseDataReadyISR() override;

            virtual void SetDataTimeout(std::chrono::milliseconds newTimeout) override;

            /**
             * @brief Initializes Sun Sensor driver.
             */
            void Initialize();

            /**
             * @brief Interrupt handler for GPIO pin.
             */
            void IRQHandler();

          private:
            /** @brief Error reporter type. */
            using ErrorReporter = error_counter::AggregatedErrorReporter<ErrorCounter::DeviceId>;

            /*
             * @brief DefaultTimeout Default timeout for measurement.
             */
            static constexpr std::chrono::milliseconds DefaultTimeout = std::chrono::seconds(8);

            /** @brief Error counter. */
            ErrorCounter errors;

            /** @brief I2C bus used to communicate with device. */
            drivers::i2c::II2CBus& i2cbus;

            /** @brief Interrupt/busy pin driver. */
            drivers::gpio::IInterruptPinDriver& interruptPinDriver;

            /** @brief Synchronization semaphore. */
            OSSemaphoreHandle sync;

            /** @brief Measurement timeout. */
            std::chrono::milliseconds dataWaitTimeout;
        };
    }
}

/** @}*/

#endif /* SRC_DEVICES_SUNS_HPP_ */
