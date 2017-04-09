#ifndef SRC_DRIVERS_BURTC_HPP_
#define SRC_DRIVERS_BURTC_HPP_

#pragma once

#include "base/os.h"

namespace devices
{
    namespace burtc
    {
        /**
         * @defgroup burtc Backup RTC Driver
         * @ingroup perhipheral_drivers
         *
         * @brief This library provides driver for built-in Backup Real Time Counter.
         *
         * @{
        */

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
            Burtc(TimeAction& tickCallback);

            /**
              * @brief Initializes Burtc device - setups the hardware and starts the FreeRTOS task.
              */
            OSResult Initialize();

            /**
             * @brief Starts ticking
             */
            void Start();

            /**
              * @brief Interrupt handler for BURTC hardware
              */
            void IRQHandler();

            /** @brief Compare value  */
            static constexpr uint32_t CompareValue = 205;

            /** @brief Prescaler divider */
            static constexpr uint32_t PrescalerDivider = 8;

          private:
            static constexpr uint32_t InterruptPriority = 6;

            TimeAction& _tickCallback;
            std::chrono::milliseconds _timeDelta;

            void ConfigureHardware();

            Task<Burtc*, 4_KB, TaskPriority::P6> _task;

            /** @brief Calculates current time interval based on selected oscillator frequency, prescaler and compare value **/
            static std::chrono::milliseconds CalculateCurrentTimeInterval();

            static void HandleTickTask(Burtc* burtcObject);
        };

        /** @} */
    }
}

#endif /* SRC_DRIVERS_BURTC_HPP_ */
