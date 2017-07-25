#ifndef LIBS_POWER_POWER_EPS_INCLUDE_EPS_POWER_EPSPOWER_C_
#define LIBS_POWER_POWER_EPS_INCLUDE_EPS_POWER_EPSPOWER_C_

#include "eps/fwd.hpp"
#include "power/power.h"

namespace services
{
    namespace power
    {
        /**
         * @brief EPS Power control
         * @ingroup power
         */
        class EPSPowerControl final : public IPowerControl
        {
          public:
            /**
             * @brief Ctor
             * @param eps EPS driver
             */
            EPSPowerControl(devices::eps::EPSDriver& eps);

            virtual void PowerCycle() override;

            virtual bool MainThermalKnife(bool enabled) override;
            virtual bool RedundantThermalKnife(bool enabled) override;
            virtual bool EnableMainSailBurnSwitch() override;
            virtual bool EnableRedundantSailBurnSwitch() override;
            virtual bool PrimaryAntennaPower(bool enabled) override;
            virtual bool BackupAntennaPower(bool enabled) override;
            virtual Option<bool> PrimaryAntennaPower() override;

          private:
            /** @brief EPS driver */
            devices::eps::EPSDriver& _eps;
        };
    }
}

#endif /* LIBS_POWER_POWER_EPS_INCLUDE_EPS_POWER_EPSPOWER_C_ */
