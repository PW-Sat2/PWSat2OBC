#ifndef LIBS_MISSION_INCLUDE_MISSION_SAIL_H_
#define LIBS_MISSION_INCLUDE_MISSION_SAIL_H_

#pragma once

#include <tuple>
#include "mission/base.hpp"
#include "power/power.h"
#include "state/struct.h"

namespace mission
{
    /**
     * @defgroup mission_sail Sail Deployment
     * @ingroup mission
     * @brief Module that contains logic related to sail deployment once the designed time has passed.
     *
     * @{
     */

    class OpenSailTask;

    struct StepDescription
    {
        void (*Action)(OpenSailTask* This);
        std::chrono::milliseconds AfterStepDelay;
    };

    /**
     * @brief Task that is responsible for deploying the sail at the end of the primary satelite mission.
     * @mission_task
     */
    class OpenSailTask : public mission::Action, public mission::Update
    {
      public:
        OpenSailTask(services::power::IPowerControl& power) : _power(power), _step(0), _nextStepAfter(0)
        {
        }

        mission::ActionDescriptor<SystemState> BuildAction();
        mission::UpdateDescriptor<SystemState> BuildUpdate();

      public:
        static UpdateResult Update(SystemState& state, void* param);
        static bool Condition(const SystemState& state, void* param);
        static void Action(SystemState& state, void* param);

        static void EnableMainThermalKnife(OpenSailTask* This)
        {
            This->_power.MainThermalKnife(true);
        }
        static void DisableMainThermalKnife(OpenSailTask* This)
        {
            This->_power.MainThermalKnife(false);
        }
        static void EnableRedundantThermalKnife(OpenSailTask* This)
        {
            This->_power.RedundantThermalKnife(true);
        }
        static void DisableRedundantThermalKnife(OpenSailTask* This)
        {
            This->_power.RedundantThermalKnife(false);
        }
        static void EnableMainBurnSwitch(OpenSailTask* This)
        {
            This->_power.EnableMainSailBurnSwitch();
        }
        static void EnableRedundantBurnSwitch(OpenSailTask* This)
        {
            This->_power.EnableRedundantSailBurnSwitch();
        }

        static StepDescription Steps[6];
        static constexpr std::uint8_t StepsCount = count_of(Steps);

        services::power::IPowerControl& _power;
        std::uint8_t _step;
        std::chrono::milliseconds _nextStepAfter;
    };

    /** @} */
}
#endif /* LIBS_MISSION_INCLUDE_MISSION_SAIL_H_ */
