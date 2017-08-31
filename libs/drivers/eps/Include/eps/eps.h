#ifndef SRC_DEVICES_EPS_H_
#define SRC_DEVICES_EPS_H_

#include <stdbool.h>
#include <cstdint>
#include "base/fwd.hpp"
#include "error_counter/error_counter.hpp"
#include "fwd.hpp"
#include "gsl/span"
#include "hk.hpp"
#include "i2c/forward.h"

namespace devices
{
    namespace eps
    {
        /**
         * @defgroup eps EPS Driver
         * @ingroup device_drivers
         *
         * @{
         */

        /**
         * @brief Interface of object capable providing complete eps telemetry.
         */
        struct IEpsTelemetryProvider
        {
            /**
             * @brief Reads housekeeping of controller A
             * @return Housekeeping of controller A
             */
            virtual Option<hk::ControllerATelemetry> ReadHousekeepingA() = 0;

            /**
             * @brief Reads housekeeping of controller B
             * @return Housekeeping of controller B
             */
            virtual Option<hk::ControllerBTelemetry> ReadHousekeepingB() = 0;
        };

        /**
         * @brief EPS driver interface
         */
        struct IEPSDriver : public IEpsTelemetryProvider
        {
            /**
             * @brief Available controller
             */
            enum class Controller
            {
                A, //!< A
                B  //!< B
            };

            /**
             * @brief Disables overheat submode on selected controller
             * @param controller Controller to use
             * @return Operation result
             */
            virtual bool DisableOverheatSubmode(Controller controller) = 0;
        };

        /**
         * @brief EPS driver
         */
        class EPSDriver final : public IEPSDriver
        {
          public:
            /** @brief Controller A address */
            static constexpr drivers::i2c::I2CAddress ControllerA = 0b0110101;
            /** @brief Controller A Id */
            static constexpr std::uint8_t ControllerAId = 0x61;

            /** @brief Controller B address */
            static constexpr drivers::i2c::I2CAddress ControllerB = 0b0110110;
            /** @brief Controller B Id */
            static constexpr std::uint8_t ControllerBId = 0x9D;

            /**
             * @brief Ctor
             * @param errorCounting Error counting mechanism
             * @param controllerABus I2C interface for controller A
             * @param controllerBBus I2C interface for controller B
             */
            EPSDriver(error_counter::ErrorCounting& errorCounting, //
                drivers::i2c::II2CBus& controllerABus,             //
                drivers::i2c::II2CBus& controllerBBus);

            /**
             * @brief Reads housekeeping of controller A
             * @return Housekeeping of controller A
             */
            virtual Option<hk::ControllerATelemetry> ReadHousekeepingA() final override;

            /**
             * @brief Reads housekeeping of controller B
             * @return Housekeeping of controller B
             */
            virtual Option<hk::ControllerBTelemetry> ReadHousekeepingB() final override;

            /**
             * @brief Performs power cycle using specified controller.
             * @param controller Controller to use
             * @return This function will return only on failure
             */
            bool PowerCycle(Controller controller);

            /**
             * @brief Performs power cycle
             * @return This function will return only on failure
             *
             * Tries to perform power cycle by controller A than on controller B
             */
            bool PowerCycle();

            /**
             * @brief Enables LCL
             * @param lcl LCL to enable
             * @return Operation result
             */
            ErrorCode EnableLCL(LCL lcl);
            /**
             * @brief Disables LCL
             * @param lcl LCL to disable
             * @return Operation result
             */
            ErrorCode DisableLCL(LCL lcl);

            /**
             * @brief Disables overheat submode on selected controller
             * @param controller Controller to use
             * @return Operation result
             */
            virtual bool DisableOverheatSubmode(Controller controller) override;

            /**
             * @brief Enables burn switch
             * @param main Use main controller (A) to enable burn switch
             * @param burnSwitch Burn switch to enable
             * @return Operation result
             */
            ErrorCode EnableBurnSwitch(bool main, BurnSwitch burnSwitch);

            /**
             * @brief Returns error code from chosen controller
             * @param controller Controller to check
             * @return Error code
             */
            ErrorCode GetErrorCode(Controller controller);

            /**
             * @brief Resets EPS watchdog timer
             * @param controller Controller on which watchdog should be reseted
             * @return Operation result
             */
            ErrorCode ResetWatchdog(Controller controller);

            /**
             * @brief Resets both watchdogs
             */
            inline void ResetWatchdog();

            /** @brief Error counter type */
            using ErrorCounter = error_counter::ErrorCounter<1>;

          private:
            /** @brief Error counter */
            ErrorCounter _error;

            /** @brief I2C interface for controller A */
            drivers::i2c::II2CBus& _controllerABus;
            /** @brief I2C interface for controller B */
            drivers::i2c::II2CBus& _controllerBBus;

            /**
             * @brief Performs write operation to selected controller
             * @param controller Controller to use
             * @param inData Data to send
             * @return Operation result
             */
            drivers::i2c::I2CResult Write(Controller controller, const gsl::span<std::uint8_t> inData);

            /**
             * @brief Performs write-read operation to selected controller
             * @param controller Controller to use
             * @param inData Data to send
             * @param outData Buffer for response
             * @return Operation result
             */
            drivers::i2c::I2CResult WriteRead(Controller controller, gsl::span<const uint8_t> inData, gsl::span<uint8_t> outData);
        };

        void EPSDriver::ResetWatchdog()
        {
            this->ResetWatchdog(Controller::A);
            this->ResetWatchdog(Controller::B);
        }

        /** @} */
    }
}

#endif /* SRC_DEVICES_EPS_H_ */
