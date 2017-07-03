#ifndef LIBS_MISSION_ANTENNA_TASK_HPP
#define LIBS_MISSION_ANTENNA_TASK_HPP

#pragma once

#include <atomic>
#include "antenna_state.h"
#include "mission/base.hpp"

namespace mission
{
    namespace antenna
    {
        /**
         * @brief Mission part related to antenna deployment.
         * @ingroup mission_atenna
         * @mission_task
         *
         * This components provides two mission fragments related to antenna deployment: Action & Update parts.
         *
         * The purpose of the Update part is to keep updating the global satellite state with current antenna
         * deployment status.
         *
         * The purpose of Action part is to coordinate the antenna deployment. This part is executed
         * only if following condition are met:
         * - The initial silent mission period is over
         * - The antennas are not currently being deployed
         */
        struct AntennaTask : public Update, public Action
        {
            /**
             * @brief ctor.
             * @param[in] driver Reference to antenna driver interface.
             */
            AntennaTask(AntennaDriver& driver);
            /**
             * @brief Returns antenna deployment action descriptor.
             *
             * @returns Action descriptor that runs antenna deployment process.
             */
            ActionDescriptor<SystemState> BuildAction();

            /**
             * @brief Returns antenna deployment update descriptor.
             *
             * @returns Update descriptor that runs antenna deployment update process.
             */
            UpdateDescriptor<SystemState> BuildUpdate();

            /**
             * @brief State of the antenna mission deployment task.
             */
            AntennaMissionState state;
        };

        /**
         * @brief Interface for object responsible for disabling antenna deployment
         * @ingroup mission_atenna
         */
        struct IDisableAntennaDeployment
        {
            /**
             * @brief Schedules antenna deployment to be disabled as soon as possible
             */
            virtual void DisableDeployment() = 0;
        };

        /**
         * @brief Stop antenna deployment task
         * @ingroup mission_atenna
         * @mission_task
         *
         * This component is responsible for disabling antenna deployment. This is achived by setting flag in persistent state.
         * Antenna deployment will be disabled if all following conditions are met:
         * * Command to stop antenna deployment has been received
         * * Antenna deployment is not already disabled
         * * Antenna deployment process has finished
         */
        class StopAntennaDeploymentTask : public mission::Action, public IDisableAntennaDeployment
        {
          public:
            /**
             * @brief Ctor
             * @param mark Dummu value
             */
            StopAntennaDeploymentTask(std::uint8_t mark);

            /**
             * @brief Return mission action
             * @return Mission action
             */
            mission::ActionDescriptor<SystemState> BuildAction();

            virtual void DisableDeployment() override;

          private:
            /**
             * @brief Mission action's condition
             * @param state System state
             * @param param Pointer to @ref StopAntennaDeploymentTask
             * @return true if all conditions for action are met
             */
            static bool Condition(const SystemState& state, void* param);

            /**
             * @brief Mission's action
             * @param state System state
             * @param param Pointer to @ref StopAntennaDeploymentTask
             */
            static void Action(SystemState& state, void* param);

            /** @brief Flag indicating whether antenna deployment should be disabled */
            std::atomic<bool> _shouldDisable{false};
        };
    }
}

#endif
