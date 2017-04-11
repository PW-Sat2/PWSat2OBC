#ifndef SRC_DEVICES_EPS_H_
#define SRC_DEVICES_EPS_H_

#include <stdbool.h>
#include <cstdint>
#include "base/reader.h"
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
         * @brief Available LCLs
         */
        enum class LCL : std::uint8_t
        {
            TKMain = 0x01,      //!< TKMain
            SunS = 0x02,        //!< SunS
            CamNadir = 0x03,    //!< CamNadir
            CamWing = 0x04,     //!< CamWing
            SENS = 0x05,        //!< SENS
            AntennaMain = 0x06, //!< AntennaMain
            TKRed = 0x11,       //!< TKRed
            AntennaRed = 0x12   //!< AntennaRed
        };

        /**
         * @brief Available BurnSwitches
         */
        enum class BurnSwitch : std::uint8_t
        {
            Sail = 0x1, //!< Sail
            SADS = 0x2  //!< SADS
        };

        /**
         * @brief Error codes
         */
        enum class ErrorCode : std::uint16_t
        {
            NoError = 0,                    //!< NoError
            OnFire = 0xAF,                  //!< OnFire
            CommunicationFailure = 0xFF + 1 //!< CommunicationFailure
        };

        /**
         * @brief EPS driver
         */
        class EPSDriver
        {
          public:
            /** @brief Controller A address */
            static constexpr drivers::i2c::I2CAddress ControllerA = 0b0110101;
            /** @brief Controller B address */
            static constexpr drivers::i2c::I2CAddress ControllerB = 0b0110110;

            /**
             * @brief Ctor
             * @param controllerABus I2C interface for controller A
             * @param controllerBBus I2C interface for controller B
             */
            EPSDriver(drivers::i2c::II2CBus& controllerABus, drivers::i2c::II2CBus& controllerBBus);

            /**
             * @brief Reads housekeeping of controller A
             * @return Housekeeping of controller A
             */
            Option<hk::HouseheepingControllerA> ReadHousekeepingA();
            /**
             * @brief Reads housekeeping of controller B
             * @return Housekeeping of controller B
             */
            Option<hk::HouseheepingControllerB> ReadHousekeepingB();

            /**
             * @brief Performs power cycle by controller A
             * @return This function will return only on failure
             */
            bool PowerCycleA();
            /**
             * @brief Performs power cycle by controller B
             * @return This function will return only on failure
             */
            bool PowerCycleB();
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
             * @brief Disables overheat submode on controller A
             * @return Operation result
             */
            bool DisableOverheatSubmodeA();
            /**
             * @brief Disables overheat submode on controller B
             * @return Operation result
             */
            bool DisableOverheatSubmodeB();

            /**
             * @brief Enables burn switch
             * @param main Use main controller (A) to enable burn switch
             * @param burnSwitch Burn switch to enable
             * @return Operation result
             */
            ErrorCode EnableBurnSwitch(bool main, BurnSwitch burnSwitch);

            /**
             * @brief Returns error code from controller A
             * @return Error code of controller A
             */
            ErrorCode GetErrorCodeA();
            /**
             * @brief Returns error code from controller B
             * @return Error code of controller B
             */
            ErrorCode GetErrorCodeB();

          private:
            /** @brief I2C interface for controller A */
            drivers::i2c::II2CBus& _controllerABus;
            /** @brief I2C interface for controller B */
            drivers::i2c::II2CBus& _controllerBBus;

            /**
             * @brief Available controller
             */
            enum class Controller
            {
                A, //!< A
                B  //!< B
            };

            /**
             * @brief Returns error code from chosen controller
             * @param controller Controller to check
             * @return Error code
             */
            ErrorCode GetErrorCode(Controller controller);

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

        /** @} */
    }
}

#endif /* SRC_DEVICES_EPS_H_ */
