#include "adcs.hpp"
#include "adcs/BuiltinDetumbling.hpp"
#include "logger/logger.h"

using adcs::BuiltinDetumbling;
using obc::Adcs;

Adcs::Adcs(devices::imtq::IImtqDriver& imtqDriver_)                                   //
    : builtinDetumbling(imtqDriver_),                                                 //
      experimentalDetumbling(imtqDriver_),                                            //
      experimentalSunpointing(imtqDriver_),                                           //
      coordinator(builtinDetumbling, experimentalDetumbling, experimentalSunpointing) //
{
}

OSResult Adcs::Initialize()
{
    auto result = coordinator.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_ERROR, "[adcs] Unable to initialize adcs coordinator. Reason: '%d'.", static_cast<int>(result));
    }

    return result;
}
