#ifndef LIBS_ADCS_ADCS_COORDINATOR_HPP
#define LIBS_ADCS_ADCS_COORDINATOR_HPP

#pragma once

#include "adcs.hpp"
#include "base/os.h"

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
        AdcsCoordinator(IDetumblingSupport& builtinDetembling_, //
            IDetumblingSupport& experimentalDetumbling_,        //
            ISunPointingSupport& sunpointAlgorithm_             //
            );

        virtual AdcsMode CurrentMode() const final override;

        virtual OSResult EnableBuiltinDetumbling() final override;

        virtual OSResult EnableExperimentalDetumbling() final override;

        virtual OSResult EnableSunPointing() final override;

        virtual OSResult Disable() final override;

      private:
        /**
         * @brief Update the state based on the pased operation status.
         *
         * @param[in] newMode Proposed new adcs operating mode.
         * @param[in] operationStatus Status of attempt to switch adcs to the new operational mode.
         * @returns Value passed as operationStatus.
         */
        OSResult SetState(AdcsMode newMode, OSResult operationStatus);

        /**
         * @brief Current adcs operational mode.
         */
        AdcsMode currentMode;

        /**
         * @brief Interface of primary adcs algorithm controller.
         */
        IDetumblingSupport& builtinDetumbling;

        /**
         * @brief Interface of experimental adcs algorithm controller.
         */
        IDetumblingSupport& experimentalDetumbling;

        /**
         * @brief Interface of sun pointing algorithm controller.
         */
        ISunPointingSupport& sunpointAlgorithm;
    };
}
#endif
