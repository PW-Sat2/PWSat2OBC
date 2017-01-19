#ifndef LIBS_ADCS_EXPERIMENT_ADCS_HPP
#define LIBS_ADCS_EXPERIMENT_ADCS_HPP

#pragma once

#include <atomic>
#include <chrono>
#include "adcs/adcs.hpp"
#include "base/os.h"
/**
 * @defgroup adcs (Draft) ADCS implementation
 *
 * @{
 */

namespace adcs
{
    /** @brief (Draft) ADCS Context */
    class AdcsExperiment final : public IDetumblingSupport, public ISunPointingSupport
    {
      public:
        AdcsExperiment();

        OSResult Initialize();

        virtual OSResult EnableSunPointing() final override;

        virtual OSResult DisableSunPointing() final override;

        virtual OSResult EnableDetumbling() final override;

        virtual OSResult DisableDetumbling() final override;

        OSResult TurnOff();

        std::chrono::milliseconds GetIterationtTime();

      private:
        /** @brief ADCS modes */
        enum class AdcsExperimentMode
        {
            Disabled,
            Detumbling,
            SunPointing
        };

        static void TaskEntry(void* arg);

        void HandleCommand();

        /** @brief Task handle */
        OSTaskHandle taskHandle;
        /** @brief Current ADCS mode */
        std::atomic<AdcsExperimentMode> currentMode;
    };
}
#endif /* LIBS_ADCS_EXPERIMENT_ADCS_HPP */
