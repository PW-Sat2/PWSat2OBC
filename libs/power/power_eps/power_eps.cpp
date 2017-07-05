#include <stdlib.h>

#include "eps/eps.h"
#include "power/power.h"
#include "power_eps.h"
#include "system.h"

using devices::eps::LCL;
using devices::eps::BurnSwitch;
using devices::eps::ErrorCode;

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

        bool EPSPowerControl::MainThermalKnife(bool enabled)
        {
            if (enabled)
            {
                return this->_eps.EnableLCL(LCL::TKMain) == ErrorCode::NoError;
            }
            else
            {
                return this->_eps.DisableLCL(LCL::TKMain) == ErrorCode::NoError;
            }
        }

        bool EPSPowerControl::RedundantThermalKnife(bool enabled)
        {
            if (enabled)
            {
                return this->_eps.EnableLCL(LCL::TKRed) == ErrorCode::NoError;
            }
            else
            {
                return this->_eps.DisableLCL(LCL::TKRed) == ErrorCode::NoError;
            }
        }

        bool EPSPowerControl::EnableMainSailBurnSwitch()
        {
            return this->_eps.EnableBurnSwitch(true, BurnSwitch::Sail) == ErrorCode::NoError;
        }

        bool EPSPowerControl::EnableRedundantSailBurnSwitch()
        {
            return this->_eps.EnableBurnSwitch(false, BurnSwitch::Sail) == ErrorCode::NoError;
        }
    }
}
