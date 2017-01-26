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

        class ExperimentContext final
        {
          public:
            ExperimentContext(OSEventGroupHandle eventGroup);
            void WaitForNextCycle();

          private:
            OSEventGroupHandle _eventGroup;
        };

        struct IExperiment
        {
            virtual Experiment Type() = 0;
            virtual void Run(ExperimentContext& context) = 0;
        };

        class MissionExperiment
        {
          public:
            MissionExperiment(void*);

            void SetExperiments(gsl::span<IExperiment*> experiments);

            void Initialize();

            void RequestExperiment(Experiment experiment);

            void BackgroundTask();

            static bool ShouldStartExperiment(const SystemState& state, void* param);

            static void StartExperiment(const SystemState& state, void* param);

            static bool ShouldKickExperiment(const SystemState& state, void* param);

            static void KickExperiment(const SystemState& state, void* param);

            struct Event
            {
                static constexpr OSEventBits InProgress = 1 << 0;
            };

          private:
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
        };

        class MissionExperimentComponent2 : public mission::Action
        {
          public:
            MissionExperimentComponent2(MissionExperiment& experimentController);

            mission::ActionDescriptor<SystemState> BuildAction();

          private:
            MissionExperiment& _experimentController;
        };
    }
}

#endif /* LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_H_ */
