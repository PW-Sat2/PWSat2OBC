#ifndef OBC_ADCS_OBC_ADCS_HPP
#define OBC_ADCS_OBC_ADCS_HPP

#pragma once

#include "adcs/AdcsCoordinator.hpp"
#include "adcs/AdcsExperimental.hpp"
#include "adcs/BuiltinDetumbling.hpp"

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
        Adcs();

        /**
         * @brief Initializes adcs subsystem.
         * @return Initialization status.
         */
        OSResult Initialize();

        /**
         * @brief Returns interface of adcs subsystem controller.
         * @return Reference to adcs subsystem controller.
         */
        adcs::IAdcsCoordinator& GetAdcsController();

      private:
        /**
         * @brief Controller of imtq based detumbling algorithm.
         */
        adcs::BuiltinDetumbling primaryDetumbling;

        /**
         * @brief Controller of experimental adcs implementation.
         */
        adcs::AdcsExperimental experimentalAdcs;

        /**
         * @brief Adcs subsystem controller.
         */
        adcs::AdcsCoordinator coordinator;
    };

    inline adcs::IAdcsCoordinator& Adcs::GetAdcsController()
    {
        return this->coordinator;
    }

    /** @} */
}

#endif
