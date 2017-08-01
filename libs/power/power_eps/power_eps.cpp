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
using EPS = devices::eps::EPSDriver::Controller;

namespace services
{
    namespace power
    {
        EPSPowerControl::EPSPowerControl(devices::eps::EPSDriver& eps) : _eps(eps), _lastPowerCycleOn(EPS::A)
        {
        }

        void EPSPowerControl::PowerCycle()
        {
            if (this->_lastPowerCycleOn == EPS::A)
            {
                this->_lastPowerCycleOn = EPS::B;
                this->_eps.PowerCycle(EPS::B);
                this->_eps.PowerCycle(EPS::A);
            }
            else
            {
                this->_lastPowerCycleOn = EPS::A;
                this->_eps.PowerCycle(EPS::A);
                this->_eps.PowerCycle(EPS::B);
            }
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

        bool EPSPowerControl::SensPower(bool enabled)
        {
            if (enabled)
            {
                return this->_eps.EnableLCL(LCL::SENS) == ErrorCode::NoError;
            }
            else
            {
                return this->_eps.DisableLCL(LCL::SENS) == ErrorCode::NoError;
            }
        }

        bool EPSPowerControl::SunSPower(bool enabled)
        {
            if (enabled)
            {
                return this->_eps.EnableLCL(LCL::SunS) == ErrorCode::NoError;
            }
            else
            {
                return this->_eps.DisableLCL(LCL::SunS) == ErrorCode::NoError;
            }
        }

        bool EPSPowerControl::CameraWing(bool enabled)
        {
            if (enabled)
            {
                return this->_eps.EnableLCL(LCL::CamWing) == ErrorCode::NoError;
            }
            else
            {
                return this->_eps.DisableLCL(LCL::CamWing) == ErrorCode::NoError;
            }
        }

        bool EPSPowerControl::CameraNadir(bool enabled)
        {
            if (enabled)
            {
                return this->_eps.EnableLCL(LCL::CamNadir) == ErrorCode::NoError;
            }
            else
            {
                return this->_eps.DisableLCL(LCL::CamNadir) == ErrorCode::NoError;
            }
        }
    }
}
