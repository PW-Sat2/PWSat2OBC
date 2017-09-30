#ifndef LIBS_ADCS_ADCS_COORDINATOR_HPP
#define LIBS_ADCS_ADCS_COORDINATOR_HPP

#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <utility>
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
         * @param[in] builtinDetumbling_ Reference to the primary detumbling algorithm controller module.
         * @param[in] experimentalDetumbling_ Reference to the backup detumbling algorithm controller module.
         * @param[in] sunpointAlgorithm_ Reference to the sun pointing algorithm controller module.
         */
        AdcsCoordinator(IAdcsProcessor& builtinDetumbling_, //
            IAdcsProcessor& experimentalDetumbling_,        //
            IAdcsProcessor& sunpointAlgorithm_              //
            );

        virtual OSResult Initialize() final override;

        virtual AdcsMode CurrentMode() const final override;

        virtual OSResult EnableBuiltinDetumbling() final override;

        virtual OSResult EnableExperimentalDetumbling() final override;

        virtual OSResult EnableSunPointing() final override;

        virtual OSResult Stop() final override;

        virtual OSResult Disable() final override;

        virtual void SetBlockMode(AdcsMode adcsMode, bool isBlocked) final override;

        /**
         * @brief Returns information whether the queried adcs mode is currently blocked.
         * @param mode Queried mode
         * @return True when requested mode is blocked, false otherwise.
         */
        bool IsModeBlocked(AdcsMode mode) const;

        /**
         * @brief Switches current adcs coordinator mode
         * @param requestedMode Requested mode
         * @return Operation status
         * @remark This procedure is not thread safe.
         */
        std::pair<AdcsMode, bool> SwitchMode(AdcsMode requestedMode);

      private:
        /**
         * @brief Adcs task entry point.
         * @param[in] arg Execution context. This pointer should point to the AdcsExperiment object type.
         */
        static void TaskEntry(AdcsCoordinator* arg);

        /**
         * @brief Iteration loop.
         */
        void Loop();

        OSResult RequestMode(AdcsMode mode);

        bool Disable(AdcsMode mode);

        std::chrono::milliseconds Run(AdcsMode mode);

        /** @brief Queue holding requested experiment */
        Queue<AdcsMode, 5> _queue;

        /**
         * @brief Current adcs operational mode.
         */
        std::atomic<AdcsMode> currentMode;

        /**
         * @brief Adcs processors.
         */
        std::array<IAdcsProcessor*, 3> adcsProcessors;

        std::array<std::atomic_bool, 3> adcsMasks;

        /** @brief Background task */
        Task<AdcsCoordinator*, 3_KB, TaskPriority::P4> _task;
    };
}
#endif
