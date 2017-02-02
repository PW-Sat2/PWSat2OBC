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
        enum class Experiment
        {
            Fibo,
            Experiment2
        };

        enum class StartResult
        {
            Success,
            Failure
        };

        enum class IterationResult
        {
            Finished,
            LoopImmediately,
            WaitForNextCycle,
            Abort
        };

        struct IExperiment
        {
            virtual Experiment Type() = 0;
            virtual StartResult Start() = 0;
            virtual IterationResult Iteration() = 0;
            virtual void Stop(IterationResult lastResult) = 0;
        };

        class MissionExperiment
        {
          public:
            MissionExperiment(void*);

            void SetExperiments(gsl::span<IExperiment*> experiments);

            void Initialize();

            void RequestExperiment(Experiment experiment);

            void AbortExperiment();

            void BackgroundTask();

            mission::ActionDescriptor<SystemState> StartExperimentAction();
            mission::ActionDescriptor<SystemState> KickExperimentAction();

            struct Event
            {
                static constexpr OSEventBits InProgress = 1 << 0;
                static constexpr OSEventBits AbortRequest = 1 << 1;
                static constexpr OSEventBits MissionLoopIterationStarted = 1 << 2;
            };

          private:
            static bool ShouldStartExperiment(const SystemState& state, void* param);

            static void StartExperiment(const SystemState& state, void* param);

            static bool ShouldKickExperiment(const SystemState& state, void* param);

            static void KickExperiment(const SystemState& state, void* param);

            gsl::span<IExperiment*> _experiments;

            OSEventGroupHandle _event;
            Queue<Experiment, 1> _queue;

            Option<Experiment> _requestedExperiment;
            Task<MissionExperiment*, 2_KB, TaskPriority::P3> _task;
        };

        class MissionExperimentComponent : public mission::Action
        {
          public:
            MissionExperimentComponent(MissionExperiment& experimentController);

            mission::ActionDescriptor<SystemState> BuildAction();

          private:
            MissionExperiment& _experimentController;
            mission::CompositeAction<SystemState, 2> _actions;
        };
    }
}

#endif /* LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_H_ */
