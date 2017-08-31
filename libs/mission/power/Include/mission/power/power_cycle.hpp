#ifndef LIBS_MISSION_POWER_INCLUDE_MISSION_POWER_POWER_CYCLE_HPP_
#define LIBS_MISSION_POWER_INCLUDE_MISSION_POWER_POWER_CYCLE_HPP_

#include <tuple>
#include "experiments/experiments.h"
#include "mission/base.hpp"
#include "power/fwd.hpp"
#include "state/struct.h"

namespace mission
{
    namespace power
    {
        struct IScrubbingStatus
        {
            virtual bool BootloaderInProgress() = 0;
            virtual bool PrimarySlotsInProgress() = 0;
            virtual bool FailsafeSlotsInProgress() = 0;
        };

        class PeriodicPowerCycleTask : public Action
        {
          public:
            PeriodicPowerCycleTask(
                std::tuple<services::power::IPowerControl&, IScrubbingStatus&, ::experiments::IExperimentController&> args);

            ActionDescriptor<SystemState> BuildAction();

          private:
            services::power::IPowerControl& _power;
            IScrubbingStatus& _scrubbingStatus;
            ::experiments::IExperimentController& _experiments;

            Option<std::chrono::milliseconds> _bootTime;

            static bool Condition(const SystemState& state, void* param);

            static void Action(SystemState& /*state*/, void* param);
        };
    }
}

#endif /* LIBS_MISSION_POWER_INCLUDE_MISSION_POWER_POWER_CYCLE_HPP_ */
