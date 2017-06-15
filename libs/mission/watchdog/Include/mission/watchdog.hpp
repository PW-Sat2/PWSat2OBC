#ifndef LIBS_MISSION_WATCHDOG_INCLUDE_MISSION_WATCHDOG_HPP_
#define LIBS_MISSION_WATCHDOG_INCLUDE_MISSION_WATCHDOG_HPP_

#include "eps/fwd.hpp"
#include "mission/base.hpp"
#include "state/fwd.hpp"

namespace mission
{
    /**
     * @defgroup mission_watchdog Watchdog
     * @ingroup mission
     */

    /**
     * @brief This task kicks device watchdog in every mission loop iteration
     * @ingroup mission_watchdog
     * @mission_task
     */
    template <typename Device> class WatchdogTask : public Action
    {
      public:
        /**
         * @brief Ctor
         * @param device Device that will be kicked
         */
        WatchdogTask(Device& device);

        /**
         * @brief Builds action description
         * @return Action descriptor
         */
        ActionDescriptor<SystemState> BuildAction();

      private:
        /**
         * @brief Kicks device watchdog
         * @param state Unused
         * @param param Pointer to task object
         */
        static void Kick(SystemState& state, void* param);

        /** @brief Device that will be kicked */
        Device& _device;
    };

    template <typename Device> WatchdogTask<Device>::WatchdogTask(Device& device) : _device(device)
    {
    }

    template <typename Device> ActionDescriptor<SystemState> WatchdogTask<Device>::BuildAction()
    {
        ActionDescriptor<SystemState> action;

        action.name = "Watchdog";
        action.condition = nullptr;
        action.actionProc = Kick;
        action.param = this;

        return action;
    }

    template <typename Device> void WatchdogTask<Device>::Kick(SystemState& /*state*/, void* param)
    {
        auto This = reinterpret_cast<WatchdogTask<Device>*>(param);

        This->_device.ResetWatchdog();
    }
}

#endif /* LIBS_MISSION_WATCHDOG_INCLUDE_MISSION_WATCHDOG_HPP_ */
