#include "adcs.hpp"
#include "logger/logger.h"

using obc::Adcs;

Adcs::Adcs() //
    : coordinator(primaryDetumbling, experimentalAdcs, experimentalAdcs)
{
}

OSResult Adcs::Initialize()
{
    const auto result = this->experimentalAdcs.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_ERROR, "[adcs] Unable to initialize experimental adcs. Reason: '%d'.", static_cast<int>(result));
    }

    return result;
}
