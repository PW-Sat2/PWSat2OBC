#ifndef SRC_DRIVERS_RTC_INCLUDE_RTC_HPP_
#define SRC_DRIVERS_RTC_INCLUDE_RTC_HPP_

#pragma once

#include "error_counter/error_counter.hpp"
#include "i2c/i2c.h"

namespace devices
{
    namespace rtc
    {
        /**
         * @defgroup rtc External Real Time Clock driver
         * @ingroup device_drivers
         *
         * @brief This library provides driver for external Real Time Clock.
         *
         * @{
        */
        struct RTCTime;
        enum class Registers;

        /**
         * @brief Interface for RTC Device.
         */
        struct IRTC
        {
            /**
              * @brief Reads entire time structure from RTC.
              * @param[out] rtcTime A reference to @ref RTCTime that will be used to store the RTC time.
              *
              * @return Transfer result - I2CResult::OK when read was successful.
              */
            virtual OSResult ReadTime(RTCTime& rtcTime) = 0;

            /**
             * @brief Checks if RTC claims that clock values are valid
             * @return true if clock values are claimed as valid
             */
            virtual bool IsIntegrityGuaranteed() = 0;

            /**
             * @brief Sets time in RTC
             * @param time RTC time
             * @return Operation result
             */
            virtual OSResult SetTime(const RTCTime& time) = 0;
        };

        /**
         * @brief RTC Device
         */
        class RTCObject : public IRTC
        {
          public:
            /**
             * @brief Constructs @ref RTCObject object
             * @param[in] errors Error counting mechanism
             * @param[in] bus A reference to I2C bus used by RTC.
             */
            RTCObject(error_counter::ErrorCounting& errors, drivers::i2c::II2CBus& bus);

            /**
             * @brief Initializes RTC device
             * @return Operation result
             */
            OSResult Initialize();

            virtual OSResult ReadTime(RTCTime& rtcTime) final override;

            virtual bool IsIntegrityGuaranteed() final override;

            virtual OSResult SetTime(const RTCTime& time) final override;

            /** @brief Address of RTC device */
            static constexpr std::uint8_t I2CAddress = 0b1010001;

            /** @brief Error counter type */
            using ErrorCounter = error_counter::ErrorCounter<2>;

          private:
            /** @brief Error reporter type */
            using ErrorReporter = error_counter::AggregatedErrorReporter<ErrorCounter::DeviceId>;

            static constexpr std::uint8_t SecondsNibbleMask = 0x70;
            static constexpr std::uint8_t MinutesNibbleMask = 0x70;
            static constexpr std::uint8_t HoursNibbleMask = 0x30;
            static constexpr std::uint8_t DaysNibbleMask = 0x30;
            static constexpr std::uint8_t MonthsNibbleMask = 0x10;
            static constexpr std::uint8_t YearsNibbleMask = 0xF0;

            ErrorCounter _error;
            drivers::i2c::II2CBus& _bus;
        };

        /**
         * @brief Structure representing time retrieved from RTC.
         */
        struct RTCTime
        {
            /** @brief Seconds 0 to 59 */
            std::uint8_t seconds;
            /** @brief Minutes 0 to 59 */
            std::uint8_t minutes;
            /** @brief Hours 0 to 23 */
            std::uint8_t hours;
            /** @brief Days 1 to 31 */
            std::uint8_t days;
            /** @brief months 1 to 12  */
            std::uint8_t months;
            /** @brief years 0 to 99  */
            std::uint8_t years;

            /**
             * @brief Calculates total seconds passed since 1900-01-01 00:00.
             * @return Time converted to total seconds.
             */
            std::chrono::seconds ToDuration() const
            {
                tm t;
                t.tm_year = 100 + years; // tm_year starts and year 1900, but RTCTime::years starts at 2000
                t.tm_mon = months - 1;
                t.tm_mday = days;
                t.tm_hour = hours;
                t.tm_min = minutes;
                t.tm_sec = seconds;
                t.tm_isdst = -1;

                return std::chrono::seconds(mktime(&t));
            }

            /**
             * @brief Checks if the RTC time is valid.
             * @return True if time is valid, false otherwise.
             */
            bool IsValid()
            {
                return seconds <= 59 //
                    && minutes <= 59 //
                    && hours <= 23   //
                    && days <= 31    //
                    && months <= 12  //
                    && years <= 99;
            }
        };

        /**
         * @brief Time register addresses available in RTC.
         */
        enum class Registers
        {
            /** @brief Seconds register **/
            VL_seconds = 0x02,
            /** @brief Minutes register **/
            minutes = 0x03,
            /** @brief Hours register **/
            hours = 0x04,
            /** @brief Days register **/
            days = 0x05,
            /** @brief Months register **/
            century_months = 0x07,
            /** @brief Years register **/
            years = 0x08
        };

        /** @} */
    }
}

#endif /* SRC_DRIVERS_RTC_INCLUDE_RTC_HPP_ */
