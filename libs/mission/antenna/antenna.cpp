#include <cstdint>
#include <cstring>
#include "antenna/driver.h"
#include "antenna/telemetry.hpp"
#include "antenna_state.h"
#include "antenna_task.hpp"
#include "gsl/gsl_util"
#include "logger/logger.h"
#include "mission/base.hpp"
#include "mission/obc.hpp"
#include "power/power.h"
#include "system.h"
#include "time/TimePoint.h"

using namespace std::chrono_literals;

namespace mission
{
    namespace antenna
    {
        std::array<AntennaTask::StepDescriptor, 12> AntennaTask::Steps = {{
            {PowerOn, AntennaChannel::ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 0s, 10s},
            {Reset, AntennaChannel::ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 0s, 60s},
            {Arm, AntennaChannel::ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 0s, 60s},
            {Deploy, AntennaChannel::ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 4 * 30s, 180s},
            {Disarm, AntennaChannel::ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 0s, 0s},
            {PowerOff, AntennaChannel::ANTENNA_PRIMARY_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 0s, 120s},
            //
            {PowerOn, AntennaChannel::ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 0s, 10s},
            {Reset, AntennaChannel::ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 0s, 60s},
            {Arm, AntennaChannel::ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 0s, 60s},
            {Deploy, AntennaChannel::ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 4 * 30s, 180s},
            {Disarm, AntennaChannel::ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 0s, 0s},
            {PowerOff, AntennaChannel::ANTENNA_BACKUP_CHANNEL, AntennaId::ANTENNA_AUTO_ID, 0s, 120s},
        }};

        AntennaTask::AntennaTask(std::tuple<IAntennaDriver&, services::power::IPowerControl&> args)
            : _powerControl(std::get<services::power::IPowerControl&>(args)), _antenna(std::get<IAntennaDriver&>(args))
        {
        }

        bool AntennaTask::Initialize()
        {
            return true;
        }

        bool AntennaTask::Condition(const SystemState& state, void* param)
        {
            auto This = reinterpret_cast<AntennaTask*>(param);

            if (state.Time < 40min)
            {
                return false;
            }

            if (This->_step >= Steps.size())
            {
                return false;
            }

            if (state.Time < This->_nextStepAt)
            {
                return false;
            }

            return true;
        }

        void AntennaTask::Action(SystemState& state, void* param)
        {
            auto This = reinterpret_cast<AntennaTask*>(param);

            (void)state;

            auto stepDescriptor = Steps[This->_step];

            stepDescriptor.Action(This, stepDescriptor.Channel, stepDescriptor.Antenna, stepDescriptor.burnTime);
            This->_step++;
            This->_nextStepAt = state.Time + stepDescriptor.waitTime;
        }

        ActionDescriptor<SystemState> AntennaTask::BuildAction()
        {
            ActionDescriptor<SystemState> descriptor;
            descriptor.name = "Deploy Antenna Action";
            descriptor.param = this;
            descriptor.condition = Condition;
            descriptor.actionProc = Action;
            return descriptor;
        }

        UpdateDescriptor<SystemState> AntennaTask::BuildUpdate()
        {
            UpdateDescriptor<SystemState> descriptor;
            descriptor.name = "Deploy Antenna Update";
            descriptor.param = this;
            //            descriptor.updateProc = AntennaDeploymentUpdate;
            return descriptor;
        }

        bool AntennaTask::GetTelemetry(devices::antenna::AntennaTelemetry& /*telemetry*/) const
        {
            //            return this->state.CurrentTelemetry(telemetry);
            return false;
        }

        StopAntennaDeploymentTask::StopAntennaDeploymentTask(std::uint8_t /*mark*/)
        {
        }

        mission::ActionDescriptor<SystemState> StopAntennaDeploymentTask::BuildAction()
        {
            mission::ActionDescriptor<SystemState> action;
            action.name = "Stop antenna deployment";
            action.param = this;
            action.condition = Condition;
            action.actionProc = Action;
            return action;
        }

        void StopAntennaDeploymentTask::DisableDeployment()
        {
            this->_shouldDisable = true;
        }

        bool StopAntennaDeploymentTask::Condition(const SystemState& state, void* param)
        {
            state::AntennaConfiguration antennaConfiguration;
            if (!state.PersistentState.Get(antennaConfiguration))
            {
                LOG(LOG_LEVEL_ERROR, "[ant] Can't get antenna configuration");
                return false;
            }

            auto This = reinterpret_cast<StopAntennaDeploymentTask*>(param);
            auto alreadyDisabled = antennaConfiguration.IsDeploymentDisabled();

            return This->_shouldDisable && !alreadyDisabled && state.AntennaState.IsDeployed();
        }

        void StopAntennaDeploymentTask::Action(SystemState& state, void*)
        {
            LOG(LOG_LEVEL_INFO, "[ant] Disabling antenna deployment");

            if (!state.PersistentState.Set(state::AntennaConfiguration(true)))
            {
                LOG(LOG_LEVEL_ERROR, "[ant] Can't set antenna configuration");
            }
        }

        OSResult AntennaTask::PowerOn(
            AntennaTask* task, AntennaChannel channel, AntennaId /*antenna*/, std::chrono::milliseconds /*burnTime*/)
        {
            bool r;
            if (channel == AntennaChannel::ANTENNA_PRIMARY_CHANNEL)
            {
                r = task->_powerControl.PrimaryAntennaPower(true);
            }
            else
            {
                r = task->_powerControl.BackupAntennaPower(true);
            }

            return r ? OSResult::Success : OSResult::DeviceNotFound;
        }

        OSResult AntennaTask::Reset(
            AntennaTask* task, AntennaChannel channel, AntennaId /*antenna*/, std::chrono::milliseconds /*burnTime*/)
        {
            return task->_antenna.Reset(channel);
        }

        OSResult AntennaTask::Arm(AntennaTask* task, AntennaChannel channel, AntennaId /*antenna*/, std::chrono::milliseconds /*burnTime*/)
        {
            return task->_antenna.Arm(channel);
        }

        OSResult AntennaTask::Deploy(AntennaTask* task, AntennaChannel channel, AntennaId antenna, std::chrono::milliseconds burnTime)
        {
            return task->_antenna.DeployAntenna(channel, antenna, burnTime, true);
        }

        OSResult AntennaTask::Disarm(
            AntennaTask* task, AntennaChannel channel, AntennaId /*antenna*/, std::chrono::milliseconds /*burnTime*/)
        {
            return task->_antenna.Disarm(channel);
        }

        OSResult AntennaTask::PowerOff(
            AntennaTask* task, AntennaChannel channel, AntennaId /*antenna*/, std::chrono::milliseconds /*burnTime*/)
        {
            bool r;
            if (channel == AntennaChannel::ANTENNA_PRIMARY_CHANNEL)
            {
                r = task->_powerControl.PrimaryAntennaPower(false);
            }
            else
            {
                r = task->_powerControl.BackupAntennaPower(false);
            }

            return r ? OSResult::Success : OSResult::DeviceNotFound;
        }
        /** @}*/
    }
}
