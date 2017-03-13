#ifndef LIBS_POWER_POWER_EPS_INCLUDE_EPS_POWER_EPSPOWER_C_
#define LIBS_POWER_POWER_EPS_INCLUDE_EPS_POWER_EPSPOWER_C_

#include "eps/eps.h"
#include "power/power.h"

namespace services
{
    namespace power
    {
        class EPSPowerControl final : public IPowerControl
        {
          public:
            EPSPowerControl(devices::eps::EPSDriver& eps);

            virtual void PowerCycle() override;

          private:
            devices::eps::EPSDriver& _eps;
        };
    }
}

#endif /* LIBS_POWER_POWER_EPS_INCLUDE_EPS_POWER_EPSPOWER_C_ */
