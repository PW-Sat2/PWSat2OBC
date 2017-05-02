#ifndef LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_HPP_
#define LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_HPP_

#include "experiments/experiments.h"
#include "mission/base.hpp"
#include "state/struct.h"

namespace mission
{
    namespace experiments
    {
        /**
         * @brief Component connecting experiments to mission loop
         * @mission_task
         */
        class MissionExperimentComponent : public mission::Action, public mission::Update
        {
          public:
            /**
             * @brief Ctor
             * @param experimentController Experiment controller
             */
            MissionExperimentComponent(::experiments::ExperimentController& experimentController);

            /**
             * @brief Returns descriptor that updates system state
             * @return Update descriptor
             */
            mission::UpdateDescriptor<SystemState> BuildUpdate();

            /**
             * @brief Builds experiment mission action
             * @return Action descriptor
             */
            mission::ActionDescriptor<SystemState> BuildAction();

            /**
             * @brief Returns start experiment mission action
             * @return Action descriptor
             */
            mission::ActionDescriptor<SystemState> StartExperimentAction();

            /**
             * @brief Returns kick experiment mission action
             * @return Action descriptor
             */
            mission::ActionDescriptor<SystemState> KickExperimentAction();

          private:
            static mission::UpdateResult UpdateSystemState(SystemState& state, void* param);

            /**
             * @brief Condition for 'Start experiment' action
             * @param state System state
             * @param param Pointer to @ref MissionExperiment
             * @return true if experiment should be started
             */
            static bool ShouldStartExperiment(const SystemState& state, void* param);

            /**
             * @brief Starts experiment
             * @param state System state
             * @param param Pointer to @ref MissionExperiment
             */
            static void StartExperiment(const SystemState& state, void* param);

            /**
             * @brief Condition for 'Kick experiment' action
             * @param state System state
             * @param param Pointer to @ref MissionExperiment
             * @return true if experiment should be kicked
             */
            static bool ShouldKickExperiment(const SystemState& state, void* param);

            /**
             * @brief Kicks experiment
             * @param state System state
             * @param param Pointer to @ref MissionExperiment
             */
            static void KickExperiment(const SystemState& state, void* param);

            /** @brief Experiment controller */
            ::experiments::ExperimentController& _experimentController;
            /** @brief Composite action descriptor for both experiment actions */
            mission::CompositeAction<SystemState, 2> _actions;
        };
    }
}

#endif /* LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_HPP_ */
