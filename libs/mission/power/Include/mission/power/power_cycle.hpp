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
        /**
         * @brief Interface for object reporting scrubbing status
         */
        struct IScrubbingStatus
        {
            /**
             * @brief Returns flag indicating whether bootloader scrubbing is in progress
             * @return true if scrubbing is in progress, false otherwise
             */
            virtual bool BootloaderInProgress() = 0;
            /**
             * @brief Returns flag indicating whether primary slots scrubbing is in progress
             * @return true if scrubbing is in progress, false otherwise
             */
            virtual bool PrimarySlotsInProgress() = 0;
            /**
             * @brief Returns flag indicating whether failsafes slots scrubbing is in progress
             * @return true if scrubbing is in progress, false otherwise
             */
            virtual bool FailsafeSlotsInProgress() = 0;
        };

        /**
         * @brief Task responsible for periodic power cycles
         * @mission_task
         *
         * Conditions for power cycle:
         *  * >= 23h since boot
         *  * No scrubbing in progress
         *  * No experiment in progress
         */
        class PeriodicPowerCycleTask : public Action, public RequireNotifyWhenTimeChanges
        {
          public:
            /**
             * @brief Ctor
             * @param args Tuple with dependencies: power control, scrubbing status, experiment controller
             */
            PeriodicPowerCycleTask(
                std::tuple<services::power::IPowerControl&, IScrubbingStatus&, ::experiments::IExperimentController&> args);

            /**
             * @brief Returns action description
             * @return Action descriptor
             */
            ActionDescriptor<SystemState> BuildAction();

            /**
             * @brief Event raised by main Mission Loop when mission time changes.
             * @param timeCorrection The time correction value. Positive - time has been advanced. Negative - time has been taken back.
             */
            void TimeChanged(std::chrono::milliseconds timeCorrection);

          private:
            /**
             * @brief Checks if power cycle conditions are met
             * @param state System state
             * @param param Pointer to this task
             * @return true if action should be executed, false otherwise
             */
            static bool Condition(const SystemState& state, void* param);

            /**
             * @brief Performs power cycle
             * @param state Not used
             * @param param Pointer to this task
             */
            static void Action(SystemState& state, void* param);

            /** @brief Power control interface */
            services::power::IPowerControl& _power;
            /** @brief Scrubbing status */
            IScrubbingStatus& _scrubbingStatus;
            /** @brief Experiments controller */
            ::experiments::IExperimentController& _experiments;

            /** @brief Boot time (timestamp at first action condition evaluation) */
            Option<std::chrono::milliseconds> _bootTime;

            /** @brief Time since boot after which power cycle should occur */
            static constexpr std::chrono::hours PowerCycleTime = std::chrono::hours(23);
        };
    }
}

#endif /* LIBS_MISSION_POWER_INCLUDE_MISSION_POWER_POWER_CYCLE_HPP_ */
