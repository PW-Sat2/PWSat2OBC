#ifndef LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_
#define LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_

#pragma once

#include <cstdint>
#include "adcs/adcs.hpp"
#include "mission/base.hpp"
#include "state/struct.h"

namespace mission
{
    namespace adcs
    {
        /**
         * @defgroup MissionADCS System state support for ADCS
         * @ingroup mission
         * @{
         */
        /**
         * @brief Primary adcs mission taks.
         * @mission_task
         *
         * This task is responsible for monitoring the adcs subsystem state and publish its state to the
         * global system state, plus it is also responsible for enabling built-in detumbling algorithm once the
         * initial silent period has passed.
         */
        class AdcsPrimaryTask : public Update, Action
        {
          public:
            /**
             * @brief ctor.
             * @param[in] adcsCoordinator Reference to the adcs subsystem controller.
             *
             * This reference is used to both monitor the adcs subsystem and to control the primary detumbling algorithm.
             */
            AdcsPrimaryTask(::adcs::IAdcsCoordinator& adcsCoordinator);

            /**
             * @brief Disables the primary detumbling algorithm.
             */
            void Disable();

            /**
             * @brief Enables the primary detumbling algorithm.
             */
            void RunDetumbling();

            /**
             * @brief Retruns information whether the primary detumbling algorithm is disabled.
             * @returns True when the algorightm is disabled, false othwerwise.
             */
            bool IsDisabled() const;

            /**
             * @brief Returns adcs primary detumbling action descriptor.
             *
             * @returns Action descriptor that runs primary adcs detumbling algorithm.
             */
            ActionDescriptor<SystemState> BuildAction();

            /**
             * @brief Returns adcs status update descriptor.
             *
             * @returns Update descriptor that runs adcs status update process.
             */
            UpdateDescriptor<SystemState> BuildUpdate();

          private:
            /**
             * @brief Procedure that verifies whether the adcs built-in detumbling process should be executed.
             * @param[in] state Pointer to global satellite state.
             * @param[in] param Pointer to the deployment condition private context. This pointer should point
             * at the object of AdcsPrimaryTask type.
             *
             * @return True if the deployment action should be performed, false otherwise.
             */
            static bool AdcsEnableBuiltinDetumblingCondition(const SystemState& state, void* param);

            /**
             * @brief This procedure is deployment action entry point.
             *
             * This procedure runs the builtin detumbling algorithm.
             * @param[in] state Pointer to global satellite state.
             * @param[in] param Pointer to the deployment condition private context. This pointer should point
             * at the object of AdcsPrimaryTask type.
             */
            static void AdcsEnableBuiltinDetumbling(SystemState& state, void* param);

            static bool IsDetumblingDisabled(const SystemState& state);

            static constexpr std::uint8_t RetryCount = 3;

            /**
             * @brief Current number of retry attempts to enable primary detumbling algorithm.
             */
            std::uint8_t retryCount;

            /**
             * @brief Access to the adcs subsystem coordinator.
             */
            ::adcs::IAdcsCoordinator& coordinator;
        };

        inline void AdcsPrimaryTask::Disable()
        {
            this->retryCount = 0;
        }

        inline void AdcsPrimaryTask::RunDetumbling()
        {
            this->retryCount = RetryCount;
        }

        inline bool AdcsPrimaryTask::IsDisabled() const
        {
            return this->retryCount == 0;
        }

        /** @} */
    }
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_ */
