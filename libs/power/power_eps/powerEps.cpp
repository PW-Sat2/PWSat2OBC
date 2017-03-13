#include <stdlib.h>

#include "eps/eps.h"
#include "power/power.h"
#include "power_eps.h"
#include "system.h"

namespace services
{
    namespace power
    {
        EPSPowerControl::EPSPowerControl(devices::eps::EPSDriver& eps) : _eps(eps)
        {
        }

        void EPSPowerControl::PowerCycle()
        {
            this->_eps.PowerCycle();
        }
    }
}
