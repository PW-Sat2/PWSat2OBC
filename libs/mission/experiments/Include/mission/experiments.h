#ifndef LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_H_
#define LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_H_

#include "base/os.h"
#include "gsl/span"
#include "mission/base.hpp"
#include "state/struct.h"

namespace mission
{
    namespace experiments
    {
        /**
         * @defgroup mission_experiments Experiments infrastructure
         * @ingroup mission
         *
         * @{
         */

        /**
         * @brief Type used to represent experiment code
         */
        using ExperimentCode = std::uint8_t;

        /**
         * @brief Experiment start result
         */
        enum class StartResult
        {
            Success, //!< Success
            Failure  //!< Failure
        };

        /**
         * @brief Experiment iteration result
         */
        enum class IterationResult
        {
            Finished,         //!< Experiment is finished
            LoopImmediately,  //!< Immediately run next iteration
            WaitForNextCycle, //!< Run next iteration when next mission loop cycle begins
            Failure           //!< Experiment failed - abort
        };

        /**
         * @brief Experiment interface
         */
        struct IExperiment
        {
            /**
             * @brief Returns experiment type
             * @return Experiment type
             */
            virtual ExperimentCode Type() = 0;
            /**
             * @brief Performs experiment start
             * @return Start result
             */
            virtual StartResult Start() = 0;

            /**
             * @brief Performs single experiment iteration
             * @return Disposition about next iteration
             */
            virtual IterationResult Iteration() = 0;

            /**
             * @brief Performs experiment clean-up
             * @param lastResult Result of last iteration
             */
            virtual void Stop(IterationResult lastResult) = 0;
        };

        /**
         * @brief Coordinates experiment execution
         */
        class MissionExperiment
        {
          public:
            /**
             * @brief Default ctor
             */
            MissionExperiment();

            /**
             * @brief Sets list of available experiments
             * @param experiments Experiments
             */
            void SetExperiments(gsl::span<IExperiment*> experiments);

            /**
             * @brief Performs initialization
             */
            void Initialize();

            /**
             * @brief Requests experiment to be started in next mission loop
             * @param experiment Experiment type
             */
            void RequestExperiment(ExperimentCode experiment);

            /**
             * @brief Aborts current experiment immediately
             */
            void AbortExperiment();

            /**
             * @brief Background task procedure
             */
            void BackgroundTask();

            /**
             * @brief Returns start experiment mission action
             * @return Action descriptor
             */
            mission::ActionDescriptor<SystemState> StartExperimentAction();

            /**
             * @brief Returns start experiment mission action
             * @return Action descriptor
             */
            mission::ActionDescriptor<SystemState> KickExperimentAction();

            /**
             * @brief Events used in synchronization
             */
            struct Event
            {
                /** @brief InProgress flag */
                static constexpr OSEventBits InProgress = 1 << 0;
                /** @brief Abort requested flag */
                static constexpr OSEventBits AbortRequest = 1 << 1;

                /** @brief Mission loop iteration started */
                static constexpr OSEventBits MissionLoopIterationStarted = 1 << 2;
            };

          private:
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

            /**
             * @brief Performs single experiment
             * @param experiment Experiment to run
             */
            void RunExperiment(IExperiment& experiment);

            /** @brief List of available experiments */
            gsl::span<IExperiment*> _experiments;

            /** @brief Event group used for synchronizatio */
            OSEventGroupHandle _event;
            /** @brief Queue holding requested experiment */
            Queue<ExperimentCode, 1> _queue;

            /** @brief Requested experiment */
            Option<ExperimentCode> _requestedExperiment;

            /** @brief Background task */
            Task<MissionExperiment*, 2_KB, TaskPriority::P3> _task;
        };

        /**
         * @brief Component connecting experiments to mission loop
         */
        class MissionExperimentComponent : public mission::Action
        {
          public:
            /**
             * @brief Ctor
             * @param experimentController Experiment controller
             */
            MissionExperimentComponent(MissionExperiment& experimentController);

            /**
             * @brief Builds experiment mission action
             * @return Action descriptor
             */
            mission::ActionDescriptor<SystemState> BuildAction();

          private:
            /** @brief Experiment controller */
            MissionExperiment& _experimentController;
            /** @brief Composite action descriptor for both experiment actions */
            mission::CompositeAction<SystemState, 2> _actions;
        };

        /** @} */
    }
}

#endif /* LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_H_ */
