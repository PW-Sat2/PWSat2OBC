#ifndef SRC_DRIVERS_BURTC_HPP_
#define SRC_DRIVERS_BURTC_HPP_

#pragma once

#include "time/timer.h"

/**
 * @defgroup burtc Backup RTC Driver
 *
 * @brief This library provides driver for built-in Backup Real Time Counter.
 *
 * @{
*/

namespace devices
{
    namespace burtc
    {
        /**
         * @brief BURTC Device
         */
        class Burtc
        {
          public:
            /**
             * @brief Constructs @ref Burtc object
             * @param[in] timeProvider Time provider that will be notified about time changes
             */
            Burtc(services::time::TimeProvider& timeProvider);

            /**
              * @brief Initializes Burtc device - setups the hardware and starts the FreeRTOS task.
              */
            void Initialize();

            /** @brief Compare value  */
            static constexpr uint32_t CompareValue = 205;

            /** @brief Prescaler divider */
            static constexpr uint32_t PrescalerDivider = 8;

            /** @brief Calculates current time interval based on selected oscilator frequency, prescaler and compare value **/
            static std::chrono::milliseconds CalculateCurrentTimeInterval();

          private:
            static constexpr uint32_t InterruptPriority = 6; // TODO: What priority?

            services::time::TimeProvider& _timeProvider;
            std::chrono::milliseconds _timeDelta;

            void ConfigureHardware();
            void StartTask();
            static void UpdateTimeProvider(void* param);
        };
    }
}

/** @} */

#endif /* SRC_DRIVERS_BURTC_HPP_ */
