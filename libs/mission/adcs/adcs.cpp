#include "adcs.hpp"
#include <algorithm>
#include "adcs/adcs.hpp"
#include "mission/obc.hpp"
#include "state/struct.h"

namespace mission
{
    namespace adcs
    {
        AdcsPrimaryTask::AdcsPrimaryTask(::adcs::IAdcsCoordinator& adcsCoordinator) //
            : retryCount(RetryCount),
              coordinator(adcsCoordinator)
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

        bool AdcsPrimaryTask::AdcsEnableBuiltinDetumblingCondition(const SystemState& state, void* param)
        {
            const auto context = static_cast<AdcsPrimaryTask*>(param);
            if (!IsInitialSilentPeriodFinished(state.Time))
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

            if (context->retryCount == 0)
            {
                return false;
            }

            return !IsDetumblingDisabled(state);
        }

        void AdcsPrimaryTask::AdcsEnableBuiltinDetumbling(SystemState& /*state*/, void* param)
        {
            const auto context = static_cast<AdcsPrimaryTask*>(param);
            const auto result = context->coordinator.EnableBuiltinDetumbling();
            if (OS_RESULT_SUCCEEDED(result))
            {
                context->retryCount = RetryCount;
            }
            else
            {
                context->retryCount = std::max(context->retryCount - 1, 0);
            }
        }

        bool AdcsPrimaryTask::IsDetumblingDisabled(const SystemState& state)
        {
            state::AdcsState adcsState;
            if (!state.PersistentState.Get(adcsState))
            {
                return true;
            }

            return adcsState.IsInternalDetumblingDisabled();
        }

        ActionDescriptor<SystemState> AdcsPrimaryTask::BuildAction()
        {
            ActionDescriptor<SystemState> descriptor;
            descriptor.name = "Enable Primary Adcs Detumbling";
            descriptor.param = this;
            descriptor.condition = AdcsEnableBuiltinDetumblingCondition;
            descriptor.actionProc = AdcsEnableBuiltinDetumbling;
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
