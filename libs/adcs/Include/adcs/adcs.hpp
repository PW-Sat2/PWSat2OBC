#ifndef LIBS_ADCS_HPP
#define LIBS_ADCS_HPP

#pragma once

#include "base/hertz.hpp"
#include "base/os.h"

namespace adcs
{
    /**
     * @defgroup adcs Adcs Support module
     *
     * This module contains components responsible for coordinating and execution of adcs algorithms.
     * @{
     */

    /**
     * @brief Enumerator for current adcs operating mode.
     */
    enum class AdcsMode
    {
        /**
         * @brief Adcs is currently disabled.
         */
        Disabled = -1,

        /**
         * @brief Currently the primary detumbling algorithm is active.
         */
        BuiltinDetumbling = 0,

        /**
         * @brief Currently the experimental detumbling algorithm is active.
         */
        ExperimentalDetumbling = 1,

        /**
         * @brief Currently the sun pointing algorithm is active.
         */
        ExperimentalSunpointing = 2,
    };

    /**
     * @brief Control interface of ADCS algorithm.
     */
    struct IAdcsProcessor
    {
        /**
         * @brief Enable method.
         * @returns Operation status.
         */
        virtual OSResult Enable() = 0;

        /**
         * @brief Disable method.
         * @returns Operation status.
         */
        virtual OSResult Disable() = 0;

        /**
         * @brief Processes ADCS algorithm.
         */
        virtual void Process() = 0;

        /**
         * @brief Frequency of calling Process method.
         */
        virtual chrono_extensions::hertz GetFrequency() const = 0;
    };

    /**
     * @brief Interface of adcs subsystem controller.
     *
     * This interface should provide a way to control all of the adcs subsystem features.
     */
    struct IAdcsCoordinator
    {
        /**
         * @brief Initializes ADCS coordinator.
         * @returns Operation status.
         */
        virtual OSResult Initialize() = 0;

        /**
         * @brief Returns current adcs subsystem operational mode.
         * @returns Current subsystem operational mode
         */
        virtual AdcsMode CurrentMode() const = 0;

        /**
         * @brief Enables primary detumbling algorithm.
         *
         * Any algorithm that may already be active will be disabled prior to enabling
         * the primary detumbling algorithm.
         * @returns Operation status.
         */
        virtual OSResult EnableBuiltinDetumbling() = 0;

        /**
         * @brief Enables experimental detumbling algorithm.
         *
         * Any algorithm that may already be active will be disabled prior to enabling
         * the backup detumbling algorithm.
         * @returns Operation status.
         */
        virtual OSResult EnableExperimentalDetumbling() = 0;

        /**
         * @brief Enables sun pointing algorithm.
         *
         * Any algorithm that may already be active will be disabled prior to enabling
         * the sun pointing algorithm.
         * @returns Operation status.
         */
        virtual OSResult EnableSunPointing() = 0;

        /**
         * @brief Disables adcs subsystem altogether.
         * @returns Operation status.
         */
        virtual OSResult Disable() = 0;
    };

    /** @} */
}

#endif
