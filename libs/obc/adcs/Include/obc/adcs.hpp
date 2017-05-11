#ifndef OBC_ADCS_OBC_ADCS_HPP
#define OBC_ADCS_OBC_ADCS_HPP

#pragma once

#include "adcs/AdcsCoordinator.hpp"
#include "adcs/BuiltinDetumbling.hpp"
#include "adcs/ExperimentalDetumbling.hpp"
#include "adcs/ExperimentalSunPointing.hpp"
#include "imtq/imtq.h"
#include "time/ICurrentTime.hpp"

namespace obc
{
    /**
     * @defgroup obc_adcs OBC adcs subsystem
     * @ingroup obc
     *
     * @{
     */

    /**
     * @brief This class ties together all adcs components into single working module.
     */
    class Adcs
    {
      public:
        /** ctor. */
        Adcs(devices::imtq::IImtqDriver& imtqDriver_, services::time::ICurrentTime& currentTime_);

        /**
         * @brief Initializes adcs subsystem.
         * @return Initialization status.
         */
        OSResult Initialize();

        /**
         * @brief Returns interface of adcs subsystem controller.
         * @return Reference to adcs subsystem controller.
         */
        adcs::IAdcsCoordinator& GetAdcsCoordinator();

      private:
        adcs::BuiltinDetumbling builtinDetumbling;
        adcs::ExperimentalDetumbling experimentalDetumbling;
        adcs::ExperimentalSunPointing experimentalSunpointing;

        /**
         * @brief Adcs subsystem controller.
         */
        adcs::AdcsCoordinator coordinator;
    };

    inline adcs::IAdcsCoordinator& Adcs::GetAdcsCoordinator()
    {
        return this->coordinator;
    }

    /** @} */
}

#endif
