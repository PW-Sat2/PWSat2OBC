#ifndef OBC_ADCS_OBC_ADCS_HPP
#define OBC_ADCS_OBC_ADCS_HPP

#pragma once

#include "adcs/AdcsCoordinator.hpp"
#include "adcs/AdcsExperiment.hpp"
#include "adcs/BuiltinDetumbling.hpp"

namespace obc
{
    class ObcAdcs
    {
      public:
        ObcAdcs();

        OSResult Initialize();

        adcs::IAdcsCoordinator& GetAdcsController();

      private:
        adcs::BuiltinDetumbling primaryDetumbling;
        adcs::AdcsExperiment experimentalAdcs;
        adcs::AdcsCoordinator coordinator;
    };

    inline adcs::IAdcsCoordinator& ObcAdcs::GetAdcsController()
    {
        return this->coordinator;
    }
}

#endif
