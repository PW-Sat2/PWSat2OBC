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

        struct IExperiment
        {
            virtual Experiment Type() = 0;
            virtual void Run() = 0;
        };

        class MissionExperiment : public mission::Action
        {
          public:
            MissionExperiment();

            void SetExperiments(gsl::span<IExperiment*> experiments);

            void Initialize();

            void RequestExperiment(Experiment experiment);

            void BackgroundTask();

            mission::ActionDescriptor<SystemState> BuildAction();

            static bool ShouldStartExperiment(const SystemState& state, void* param);

            static void StartExperiment(const SystemState& state, void* param);

            struct Event
            {
                static constexpr OSEventBits InProgress = 1 << 0;
            };

          private:
            gsl::span<IExperiment*> _experiments;

            OSEventGroupHandle _event;
            Queue<Experiment, 1> _queue;

            Option<Experiment> _requestedExperiment;
        };
    }
}

#endif /* LIBS_MISSION_EXPERIMENTS_INCLUDE_MISSION_EXPERIMENTS_H_ */
