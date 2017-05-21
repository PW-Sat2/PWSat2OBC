namespace drivers
{
    namespace watchdog
    {
        /**
         * @brief Driver for external watchdog
         * @ingroup watchdog
         */
        class ExternalWatchdog
        {
          public:
            /** @brief Enables watchdog */
            static void Enable();
            /** @brief Disables watchdog */
            static void Disable();
            /** @brief Kicks watchdog */
            static void Kick();
        };
    }
}
