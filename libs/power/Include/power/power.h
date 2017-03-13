#ifndef POWER_H_
#define POWER_H_

#include "system.h"

EXTERNC_BEGIN

/**
 * @defgroup power Power Control interface
 * @{
 */

namespace services
{
    namespace power
    {
        struct IPowerControl
        {
            virtual void PowerCycle() = 0;
        };
    }
}

/** @} */

EXTERNC_END

#endif /* POWER_H_ */
