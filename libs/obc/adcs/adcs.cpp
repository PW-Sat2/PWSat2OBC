#include "adcs.hpp"
#include "adcs/BuiltinDetumbling.hpp"
#include "logger/logger.h"

using adcs::BuiltinDetumbling;
using obc::Adcs;

Adcs::Adcs(devices::imtq::IImtqDriver& imtqDriver_, services::power::IPowerControl& power) //
    : builtinDetumbling(imtqDriver_, power),                                               //
      coordinator(builtinDetumbling)                                                       //
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
