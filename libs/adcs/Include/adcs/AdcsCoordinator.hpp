#ifndef LIBS_ADCS_ADCS_COORDINATOR_HPP
#define LIBS_ADCS_ADCS_COORDINATOR_HPP

#pragma once

#include <array>
#include <chrono>
#include "adcs.hpp"
#include "base/os.h"
#include "time/ICurrentTime.hpp"

namespace adcs
{
    /**
     * @brief Class responsible for coordinating the execution state of
     * all supported adcs algorithms.
     *
     * In this version this class can be used to control two detumbling algorithms and one sun pointing algorithm.
     * @ingroup adcs
     */
    class AdcsCoordinator final : public IAdcsCoordinator
    {
      public:
        /**
         * @brief ctor.
         * @param[in] builtinDetembling_ Reference to the primary detumbling algorithm controller module.
         * @param[in] experimentalDetumbling_ Reference to the backup detumbling algorithm controller module.
         * @param[in] sunpointAlgorithm_ Reference to the sun pointing algorithm controller module.
         */
        AdcsCoordinator(IAdcsProcessor& builtinDetumbling_, //
            IAdcsProcessor& experimentalDetumbling_,        //
            IAdcsProcessor& sunpointAlgorithm_,             //
            services::time::ICurrentTime& currentTime_      //
            );

        virtual OSResult Initialize() final override;

        virtual AdcsMode CurrentMode() const final override;

        virtual OSResult EnableBuiltinDetumbling() final override;

        virtual OSResult EnableExperimentalDetumbling() final override;

        virtual OSResult EnableSunPointing() final override;

        virtual OSResult Disable() final override;

      private:
        /**
         * @brief Adcs task entry point.
         * @param[in] arg Execution context. This pointer should point to the AdcsExperiment object type.
         */
        static void TaskEntry(void* arg);

        /**
         * @brief Update the state based on the pased operation status.
         *
         * @param[in] newMode Proposed new adcs operating mode.
         * @param[in] operationStatus Status of attempt to switch adcs to the new operational mode.
         * @returns Value passed as operationStatus.
         */
        OSResult SetState(AdcsMode newMode, OSResult operationStatus);

        /**
         * @brief Enables given algorithm.
         *
         * Any algorithm that may already be active will be disabled prior to enabling
         * the given algorithm.
         * @param[in] mode New adcs operating mode.
         * @returns Operation status.
         */
        OSResult Enable(AdcsMode mode);

        /**
         * @brief Disables adcs subsystem.
         *
         * @param[in] suspend Whether to suspend adcs task.
         * @returns Operation status.
         */
        OSResult Disable(bool suspend);

        /**
         * @brief Iteration loop.
         */
        void Loop();

        /** @brief Task handle. */
        OSTaskHandle taskHandle;

        /** @brief Current time. */
        services::time::ICurrentTime& currentTime;

        /** @brief Semaphore used for task synchronization. */
        OSSemaphoreHandle sync;

        /**
         * @brief Current adcs operational mode.
         */
        AdcsMode currentMode;

        /**
         * @brief Adcs processors.
         */
        std::array<IAdcsProcessor*, 3> adcsProcessors;
    };
}
#endif
