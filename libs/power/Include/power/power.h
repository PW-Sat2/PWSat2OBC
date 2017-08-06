#ifndef POWER_H_
#define POWER_H_

#include "system.h"
#include "utils.h"

namespace services
{
    namespace power
    {
        /**
         * @defgroup power Power Control interface
         * @{
         */

        /**
         * @brief Power control API
         */
        struct IPowerControl
        {
            /**
             * @brief Performs power cycle
             */
            virtual void PowerCycle() = 0;

            /**
             * @brief Sets state of main thermal knife LCL
             * @param enabled true to turn on LCL, false to turn off
             * @return Operation status
             */
            virtual bool MainThermalKnife(bool enabled) = 0;
            /**
             * @brief Sets state of redundant thermal knife LCL
             * @param enabled true to turn on LCL, false to turn off
             * @return Operation status
             */
            virtual bool RedundantThermalKnife(bool enabled) = 0;

            /**
             * @brief Enables main sail burn switch
             * @return Operation status
             */
            virtual bool EnableMainSailBurnSwitch() = 0;

            /**
             * @brief Enables redundant sail burn switch
             * @return Operation status
             */
            virtual bool EnableRedundantSailBurnSwitch() = 0;

            /**
            * @brief Sets state of primary antenna power LCL
            * @param enabled true to turn on LCL, false to turn off
            * @return Operation status
            */
            virtual bool PrimaryAntennaPower(bool enabled) = 0;

            /**
             * @brief Checks if primary antenna power is enabled
             * @return true if module is powered on, false otherwise
             */
            virtual Option<bool> PrimaryAntennaPower() = 0;

            /**
             * @brief Sets state of backup antenna power LCL
             * @param enabled true to turn on LCL, false to turn off
             * @return Operation status
             */
            virtual bool BackupAntennaPower(bool enabled) = 0;

            /**
             * @brief Sets state of SENS LCL
             * @param  enabled true to turn on LCL, false to turn off
             * @return Operation status
             */
            virtual bool SensPower(bool enabled) = 0;

            /**
             * @brief Sets state of Experimental SunS LCL
             * @param enabled true to turn on LCL, false to turn off
             * @return Operation status
             */
            virtual bool SunSPower(bool enabled) = 0;
        };

        /** @} */
    }
}

#endif /* POWER_H_ */
