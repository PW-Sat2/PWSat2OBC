#ifndef LIBS_ADCS_EXPERIMENT_ADCS_HPP
#define LIBS_ADCS_EXPERIMENT_ADCS_HPP

#pragma once

#include <atomic>
#include <chrono>
#include "adcs/adcs.hpp"
#include "base/os.h"

namespace adcs
{
    /**
     * @brief This class controls both backup detumbling & sun pointing algorithms.
     * @ingroup adcs
     *
     * This module creates its own system task that executes the requested algorithm in it completely asynchronously
     * in regards to the rest of the system.
     */
    class AdcsExperiment final : public IDetumblingSupport, public ISunPointingSupport
    {
      public:
        /** @brief ctor. */
        AdcsExperiment();

        /**
         * @brief Initializes this module.
         * @returns Operation status.
         */
        OSResult Initialize();

        virtual OSResult EnableSunPointing() final override;

        virtual OSResult DisableSunPointing() final override;

        virtual OSResult EnableDetumbling() final override;

        virtual OSResult DisableDetumbling() final override;

        /**
         * @brief Disables this module.
         * @returns Operation status.
         */
        OSResult TurnOff();

        /**
         * @brief Returns length of single adcs algorithm iteration in milliseconds.
         * @returns Length of single algorithm iteration.
         */
        std::chrono::milliseconds GetIterationtTime();

      private:
        /** @brief ADCS modes */
        enum class AdcsExperimentMode
        {
            Disabled,
            Detumbling,
            SunPointing
        };

        /**
         * @brief Adcs task entry point.
         * @param[in] arg Execution context. This pointer should point to the AdcsExperiment object type.
         */
        static void TaskEntry(void* arg);

        /**
         * @brief Performs sinngle algorithm iteration.
         */
        void HandleCommand();

        /** @brief Task handle */
        OSTaskHandle taskHandle;

        /** @brief Current ADCS mode */
        std::atomic<AdcsExperimentMode> currentMode;
    };
}

#endif /* LIBS_ADCS_EXPERIMENT_ADCS_HPP */
