#ifndef LIBS_ADCS_HPP
#define LIBS_ADCS_HPP

#pragma once

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
        Disabled,

        /**
         * @brief Currently the primary detumbling algorithm is active.
         */
        BuiltinDetumbling,

        /**
         * @brief Currently the backup detumbling algorithm is active.
         */
        CustomDetumbling,

        /**
         * @brief Currently the sun pointing algorithm is active.
         */
        CustomSunpointing,
    };

    /**
     * @brief Controll interface of detumbling algorithm.
     */
    struct IDetumblingSupport
    {
        /**
         * @brief Enables the current detumbling algorithm.
         * @returns Operation status.
         */
        virtual OSResult EnableDetumbling() = 0;

        /**
         * @brief Disables the current detumbling algorithm.
         * @returns Operation status.
         */
        virtual OSResult DisableDetumbling() = 0;
    };

    /**
     * @brief Control interface of sun pointing algorithm.
     */
    struct ISunPointingSupport
    {
        /**
         * @brief Enables the current sun pointing algorithm.
         * @returns Operation status.
         */
        virtual OSResult EnableSunPointing() = 0;

        /**
         * @brief Disables the current sun pointing algorithm.
         * @returns Operation status.
         */
        virtual OSResult DisableSunPointing() = 0;
    };

    /**
     * @brief Interface of adcs subsystem controller.
     *
     * This interface should provide a way to control all of the adcs subsystem features.
     */
    struct IAdcsCoordinator
    {
        /**
         * @brief Rreturns current adcs subsystem operational mode.
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
         * @brief Enables backup detumbling algorithm.
         *
         * Any algorithm that may already be active will be disabled prior to enabling
         * the backup detumbling algorithm.
         * @returns Operation status.
         */
        virtual OSResult EnableCustomDetumbling() = 0;

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
