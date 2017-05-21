#ifndef LIBS_DRIVERS_WATCHDOG_INCLUDE_WATCHDOG_WATCHDOG_HPP_
#define LIBS_DRIVERS_WATCHDOG_INCLUDE_WATCHDOG_WATCHDOG_HPP_

namespace drivers
{
    namespace watchdog
    {
        /**
         * @brief Driver for internal (MCU) watchdog
         * @ingroup watchdog
         */
        class InternalWatchdog
        {
          public:
            /** @brief Enables watchdog */
            static void Enable();
            /** @brief Disables watchdog */
            static void Disable();
            /** @brief Kicks watchdog */
            static void Kick();
        };

        /** @} */
    }
}

#endif /* LIBS_DRIVERS_WATCHDOG_INCLUDE_WATCHDOG_WATCHDOG_HPP_ */
