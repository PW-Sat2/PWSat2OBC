#include <stdlib.h>

#include "eps/eps.h"
#include "power/power.h"
#include "power_eps.h"
#include "system.h"

using devices::eps::LCL;
using devices::eps::BurnSwitch;

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

        void EPSPowerControl::OpenSail()
        {
            this->_eps.EnableLCL(LCL::TKMain);
            this->_eps.EnableBurnSwitch(true, BurnSwitch::Sail);
            // TODO: Correct implementation
        }
    }
}
