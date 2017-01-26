#include "adcs.hpp"
#include "adcs/adcs.hpp"
#include "mission/obc.hpp"
#include "state/struct.h"

namespace mission
{
    namespace adcs
    {
        constexpr std::uint8_t RetryCount = 3;

        AdcsPrimaryTask::AdcsPrimaryTask(::adcs::IAdcsCoordinator& adcsCoordinator) //
            : retryCount(RetryCount),
              finished(true),
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

        bool AdcsPrimaryTask::AdcsEnablePrimaryDetumblingCondition(const SystemState& state, void* param)
        {
            const auto context = static_cast<AdcsPrimaryTask*>(param);
            if (!IsInitialSilenPeriodFinished(state.Time))
            {
                return false;
            }

            if (context->coordinator.CurrentMode() != ::adcs::AdcsMode::Disabled)
            {
                return false;
            }

            if (!context->finished)
            {
                return false;
            }

            return true;
        }

        void AdcsPrimaryTask::AdcsEnablePrimaryDetumbling(const SystemState& /*state*/, void* param)
        {
            const auto context = static_cast<AdcsPrimaryTask*>(param);
            const auto result = context->coordinator.EnableBuiltinDetumbling();
            context->finished = OS_RESULT_SUCCEEDED(result);
            if (OS_RESULT_SUCCEEDED(result))
            {
                context->retryCount = 3;
            }
        }

        ActionDescriptor<SystemState> AdcsPrimaryTask::BuildAction()
        {
            ActionDescriptor<SystemState> descriptor;
            descriptor.name = "Enable Primary Adcs Detumbling";
            descriptor.param = this;
            descriptor.condition = AdcsEnablePrimaryDetumblingCondition;
            descriptor.actionProc = AdcsEnablePrimaryDetumbling;
            return descriptor;
        }

        UpdateDescriptor<SystemState> AdcsPrimaryTask::BuildUpdate()
        {
            UpdateDescriptor<SystemState> descriptor;
            descriptor.name = "Adcs Status Update";
            descriptor.param = this;
            descriptor.updateProc = AdcsStatusUpdate;
            return descriptor;
        }
    }
}
