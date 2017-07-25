#include <stdlib.h>

#include "eps/eps.h"
#include "logger/logger.h"
#include "power/power.h"
#include "power_eps.h"
#include "system.h"

using devices::eps::LCL;
using devices::eps::hk::DISTR_LCL_STATE;
using devices::eps::hk::DISTR_LCL_FLAGB;
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

        bool EPSPowerControl::PrimaryAntennaPower(bool enabled)
        {
            if (enabled)
            {
                return this->_eps.EnableLCL(LCL::AntennaMain) == ErrorCode::NoError;
            }
            else
            {
                return this->_eps.DisableLCL(LCL::AntennaMain) == ErrorCode::NoError;
            }
        }

        bool EPSPowerControl::BackupAntennaPower(bool enabled)
        {
            if (enabled)
            {
                return this->_eps.EnableLCL(LCL::AntennaRed) == ErrorCode::NoError;
            }
            else
            {
                return this->_eps.DisableLCL(LCL::AntennaRed) == ErrorCode::NoError;
            }
        }

        Option<bool> EPSPowerControl::PrimaryAntennaPower()
        {
            auto hk = this->_eps.ReadHousekeepingA();

            if (!hk.HasValue)
            {
                return None<bool>();
            }

            auto state = has_flag(hk.Value.distr.LCL_STATE, DISTR_LCL_STATE::Antenna);
            auto flagb = has_flag(hk.Value.distr.LCL_FLAGB, DISTR_LCL_FLAGB::Antenna);

            return Some(state && flagb);
        }
    }
}
