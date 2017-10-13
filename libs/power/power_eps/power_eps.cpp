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
            return Switch(enabled, LCL::TKMain);
        }

        bool EPSPowerControl::RedundantThermalKnife(bool enabled)
        {
            return Switch(enabled, LCL::TKRed);
        }

        bool EPSPowerControl::EnableMainSailBurnSwitch()
        {
            return this->_eps.EnableBurnSwitch(true, BurnSwitch::Sail) == ErrorCode::NoError;
        }

        bool EPSPowerControl::EnableRedundantSailBurnSwitch()
        {
            return this->_eps.EnableBurnSwitch(false, BurnSwitch::Sail) == ErrorCode::NoError;
        }

        bool EPSPowerControl::EnableMainSADSBurnSwitch()
        {
            return this->_eps.EnableBurnSwitch(true, BurnSwitch::SADS) == ErrorCode::NoError;
        }

        bool EPSPowerControl::EnableRedundantSADSBurnSwitch()
        {
            return this->_eps.EnableBurnSwitch(false, BurnSwitch::SADS) == ErrorCode::NoError;
        }

        bool EPSPowerControl::PrimaryAntennaPower(bool enabled)
        {
            return Switch(enabled, LCL::AntennaMain);
        }

        bool EPSPowerControl::BackupAntennaPower(bool enabled)
        {
            return Switch(enabled, LCL::AntennaRed);
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
            return Switch(enabled, LCL::SENS);
        }

        bool EPSPowerControl::SunSPower(bool enabled)
        {
            return Switch(enabled, LCL::SunS);
        }

        bool EPSPowerControl::CameraWing(bool enabled)
        {
            return Switch(enabled, LCL::CamWing);
        }

        bool EPSPowerControl::CameraNadir(bool enabled)
        {
            return Switch(enabled, LCL::CamNadir);
        }

        bool EPSPowerControl::Switch(bool enabled, LCL lcl)
        {
            if (enabled)
            {
                return this->_eps.EnableLCL(lcl) == ErrorCode::NoError;
            }
            else
            {
                return this->_eps.DisableLCL(lcl) == ErrorCode::NoError;
            }
        }

        bool EPSPowerControl::IgnoreOverheat()
        {
            auto a = this->_eps.DisableOverheatSubmode(EPS::A);
            auto b = this->_eps.DisableOverheatSubmode(EPS::B);

            return a && b;
        }
    }
}
