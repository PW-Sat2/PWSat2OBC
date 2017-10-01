#include <cstdint>
#include <cstring>
#include <tuple>
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
        template <typename... Elements, std::size_t... Indexes, typename Element = std::tuple_element_t<0, std::tuple<Elements...>>>
        static constexpr std::array<Element, sizeof...(Elements)> ToArray(
            std::tuple<Elements...> t, std::integer_sequence<std::size_t, Indexes...> /*i*/)
        {
            return {std::get<Indexes>(t)...};
        }

        template <typename... Tuples> static decltype(auto) Join(const Tuples... tuples)
        {
            auto joined = std::tuple_cat(tuples...);

            constexpr auto TupleSize = std::tuple_size<decltype(joined)>::value;

            return ToArray(joined, std::make_index_sequence<TupleSize>());
        }

        template <AntennaChannel Channel> struct Step
        {
            static constexpr AntennaTask::StepDescriptor PowerOn = {AntennaTask::PowerOn, Channel, AntennaId::ANTENNA_AUTO_ID, 0s, 10s};

            static constexpr AntennaTask::StepDescriptor Reset = {AntennaTask::Reset, Channel, AntennaId::ANTENNA_AUTO_ID, 0s, 60s};

            static constexpr AntennaTask::StepDescriptor Arm = {AntennaTask::Arm, Channel, AntennaId::ANTENNA_AUTO_ID, 0s, 60s};

            static constexpr AntennaTask::StepDescriptor AutoDeploy = {
                AntennaTask::Deploy, Channel, AntennaId::ANTENNA_AUTO_ID, 4 * 30s, 180s};

            template <AntennaId Antenna>
            static constexpr AntennaTask::StepDescriptor ManualDeploy = {AntennaTask::Deploy, Channel, Antenna, 30s, 90s};

            static constexpr AntennaTask::StepDescriptor Disarm = {AntennaTask::Disarm, Channel, AntennaId::ANTENNA_AUTO_ID, 0s, 0s};

            static constexpr AntennaTask::StepDescriptor PowerOff = {AntennaTask::PowerOff, Channel, AntennaId::ANTENNA_AUTO_ID, 0s, 120s};

            static constexpr auto FullSequenceAuto = std::make_tuple(PowerOn, Reset, Arm, AutoDeploy, Disarm, PowerOff);

            template <AntennaId Antenna>
            static constexpr auto FullSequenceManual = std::make_tuple(PowerOn, Reset, Arm, ManualDeploy<Antenna>, Disarm, PowerOff);
        };

        decltype(AntennaTask::Steps) AntennaTask::Steps = Join( //
            Step<AntennaChannel::ANTENNA_PRIMARY_CHANNEL>::FullSequenceAuto,
            Step<AntennaChannel::ANTENNA_PRIMARY_CHANNEL>::FullSequenceManual<AntennaId::ANTENNA1_ID>,
            Step<AntennaChannel::ANTENNA_PRIMARY_CHANNEL>::FullSequenceManual<AntennaId::ANTENNA2_ID>,
            Step<AntennaChannel::ANTENNA_PRIMARY_CHANNEL>::FullSequenceManual<AntennaId::ANTENNA3_ID>,
            Step<AntennaChannel::ANTENNA_PRIMARY_CHANNEL>::FullSequenceManual<AntennaId::ANTENNA4_ID>,
            Step<AntennaChannel::ANTENNA_BACKUP_CHANNEL>::FullSequenceAuto,
            Step<AntennaChannel::ANTENNA_BACKUP_CHANNEL>::FullSequenceManual<AntennaId::ANTENNA1_ID>,
            Step<AntennaChannel::ANTENNA_BACKUP_CHANNEL>::FullSequenceManual<AntennaId::ANTENNA2_ID>,
            Step<AntennaChannel::ANTENNA_BACKUP_CHANNEL>::FullSequenceManual<AntennaId::ANTENNA3_ID>,
            Step<AntennaChannel::ANTENNA_BACKUP_CHANNEL>::FullSequenceManual<AntennaId::ANTENNA4_ID>);

        AntennaTask::AntennaTask(std::tuple<IAntennaDriver&, services::power::IPowerControl&> args)
            : _powerControl(std::get<services::power::IPowerControl&>(args)), _antenna(std::get<IAntennaDriver&>(args)), _step(0),
              _nextStepAt(0)
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

            state::AntennaConfiguration cfg;

            if (state.PersistentState.Get(cfg))
            {
                if (cfg.IsDeploymentDisabled())
                {
                    return false;
                }
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
