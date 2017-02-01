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
         * @brief Interface for callback objects that will receive ticks.
         */
        struct BurtcTickCallback
        {
            /**
             * @brief Method that will be called by BURTC.
             * @param[in] interval Interval that passed since last tick
             */
            void virtual Tick(std::chrono::milliseconds interval) = 0;
        };

        /**
         * @brief BURTC Device
         */
        class Burtc
        {
          public:
            /**
             * @brief Constructs @ref Burtc object
             * @param[in] tickCallback A pointer to function that will be notified about time changes
             */
            Burtc(BurtcTickCallback& tickCallback);

            /**
              * @brief Initializes Burtc device - setups the hardware and starts the FreeRTOS task.
              */
            void Initialize();

            /** @brief Compare value  */
            static constexpr uint32_t CompareValue = 205;

            /** @brief Prescaler divider */
            static constexpr uint32_t PrescalerDivider = 8;

          private:
            static constexpr uint32_t InterruptPriority = 6; // TODO: What priority?

            BurtcTickCallback& _tickCallback;
            std::chrono::milliseconds _timeDelta;

            void ConfigureHardware();
            void StartTask();

            /** @brief Calculates current time interval based on selected oscilator frequency, prescaler and compare value **/
            static std::chrono::milliseconds CalculateCurrentTimeInterval();

            static void HandleTickTask(void* param);
        };
    }
}

/** @} */

#endif /* SRC_DRIVERS_BURTC_HPP_ */
