#include "adcs.hpp"
#include <algorithm>
#include "adcs/adcs.hpp"
#include "mission/obc.hpp"
#include "state/struct.h"

using namespace std::chrono_literals;

namespace mission
{
    namespace adcs
    {
        AdcsPrimaryTask::AdcsPrimaryTask(::adcs::IAdcsCoordinator& adcsCoordinator) //
            : CompositeAction("ADCS Task", BuildStartAction(), BuildStopAction()),  //
              retryCount(RetryCount),
              coordinator(adcsCoordinator),
              state(State::WaitingForStart)
        {
        }
        /**
         * @brief This procedure is adcs status update descriptor entry point.
         *
         * This procedure updates the global satellite state with current adcs subsystem state.
         * @param[in] state Pointer to global satellite state.
         * @param[in] param Pointer to the deployment condition private context. This pointer should point
         * at the object of adcs::IAdcsCoordinator type.
         * @return Operation status.
         */
        static UpdateResult AdcsStatusUpdate(SystemState& state, void* param)
        {
            const auto context = static_cast<::adcs::IAdcsCoordinator*>(param);
            state.AdcsMode = context->CurrentMode();
            return UpdateResult::Ok;
        }

        bool AdcsPrimaryTask::StartCondition(const SystemState& state, void* param)
        {
            const auto context = static_cast<AdcsPrimaryTask*>(param);
            if (!IsInitialSilentPeriodFinished(state.Time))
            {
                return false;
            }

            if (state.Time > 4h)
            {
                return false;
            }

            if (context->coordinator.CurrentMode() != ::adcs::AdcsMode::Stopped)
            {
                return false;
            }

            if (!state.AntennaState.IsDeployed())
            {
                return false;
            }

            if (IsDetumblingDisabled(state))
            {
                return false;
            }

            return true; // !IsDetumblingDisabled(state);
        }

        void AdcsPrimaryTask::Start(SystemState& /*state*/, void* param)
        {
            const auto context = static_cast<AdcsPrimaryTask*>(param);
            const auto result = context->coordinator.EnableBuiltinDetumbling();
            if (OS_RESULT_SUCCEEDED(result))
            {
                context->state = State::Detumbling;
            }
        }

        bool AdcsPrimaryTask::StopCondition(const SystemState& state, void* param)
        {
            const auto context = static_cast<AdcsPrimaryTask*>(param);

            if (state.Time <= 4h)
            {
                return false;
            }

            if (context->state != State::Detumbling)
            {
                return false;
            }

            if (context->coordinator.CurrentMode() == ::adcs::AdcsMode::Stopped)
            {
                return false;
            }

            return true;
        }

        void AdcsPrimaryTask::Stop(SystemState& state, void* param)
        {
            (void)state;

            const auto context = static_cast<AdcsPrimaryTask*>(param);

            if (OS_RESULT_SUCCEEDED(context->coordinator.Stop()))
            {
                context->state = State::Stopped;
            }
        }

        bool AdcsPrimaryTask::IsDetumblingDisabled(const SystemState& state)
        {
            state::AdcsState adcsState;
            if (!state.PersistentState.Get(adcsState))
            {
                return false;
            }

            return adcsState.IsInternalDetumblingDisabled();
        }

        ActionDescriptor<SystemState> AdcsPrimaryTask::BuildStartAction()
        {
            ActionDescriptor<SystemState> descriptor;
            descriptor.name = "Start Detumbling";
            descriptor.param = this;
            descriptor.condition = StartCondition;
            descriptor.actionProc = Start;
            return descriptor;
        }

        ActionDescriptor<SystemState> AdcsPrimaryTask::BuildStopAction()
        {
            ActionDescriptor<SystemState> descriptor;
            descriptor.name = "Stop Detumbling";
            descriptor.param = this;
            descriptor.condition = StopCondition;
            descriptor.actionProc = Stop;
            return descriptor;
        }

        UpdateDescriptor<SystemState> AdcsPrimaryTask::BuildUpdate()
        {
            UpdateDescriptor<SystemState> descriptor;
            descriptor.name = "Adcs Status Update";
            descriptor.param = &this->coordinator;
            descriptor.updateProc = AdcsStatusUpdate;
            return descriptor;
        }
    }
}
