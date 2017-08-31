#ifndef LIBS_MISSION_POWER_INCLUDE_MISSION_POWER_POWER_CYCLE_HPP_
#define LIBS_MISSION_POWER_INCLUDE_MISSION_POWER_POWER_CYCLE_HPP_

#include "mission/base.hpp"
#include "power/fwd.hpp"
#include "state/struct.h"

namespace mission
{
    namespace power
    {
        class PeriodicPowerCycleTask : public Action
        {
          public:
            PeriodicPowerCycleTask(services::power::IPowerControl& power);

            ActionDescriptor<SystemState> BuildAction();

          private:
            services::power::IPowerControl& _power;

            Option<std::chrono::milliseconds> _bootTime;

            static bool Condition(const SystemState& state, void* param);

            static void Action(SystemState& /*state*/, void* param);
        };
    }
}

#endif /* LIBS_MISSION_POWER_INCLUDE_MISSION_POWER_POWER_CYCLE_HPP_ */
