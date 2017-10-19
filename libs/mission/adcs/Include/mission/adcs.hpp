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
        class AdcsPrimaryTask : public Update, public CompositeAction<SystemState, 2>
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
             * @brief Returns action descriptor for starting detumbling.
             *
             * @returns Action descriptor that runs adcs detumbling algorithm.
             */
            ActionDescriptor<SystemState> BuildStartAction();

            /**
              * @brief Returns action descriptor for stop detumbling.
              *
              * @returns Action descriptor that stops adcs detumbling algorithm.
              */
            ActionDescriptor<SystemState> BuildStopAction();

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
            static bool StartCondition(const SystemState& state, void* param);

            /**
             * @brief This procedure is deployment action entry point.
             *
             * This procedure runs the builtin detumbling algorithm.
             * @param[in] state Pointer to global satellite state.
             * @param[in] param Pointer to the deployment condition private context. This pointer should point
             * at the object of AdcsPrimaryTask type.
             */
            static void Start(SystemState& state, void* param);

            static bool StopCondition(const SystemState& state, void* param);
            static void Stop(SystemState& state, void* param);

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

            enum class State
            {
                WaitingForStart,
                Detumbling,
                Stopped
            };

            State state;
        };

        /** @} */
    }
}

#endif /* LIBS_MISSION_INCLUDE_MISSION_ADCS_MISSION_H_ */
