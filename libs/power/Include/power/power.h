#ifndef POWER_H_
#define POWER_H_

#include "system.h"

EXTERNC_BEGIN

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
             * @brief Performs sail opening sequence
             */
            virtual void OpenSail() = 0;
        };

        /** @} */
    }
}

EXTERNC_END

#endif /* POWER_H_ */
