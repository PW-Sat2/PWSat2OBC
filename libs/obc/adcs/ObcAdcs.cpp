#include "ObcAdcs.hpp"
#include "logger/logger.h"

using obc::ObcAdcs;

ObcAdcs::ObcAdcs() //
    : coordinator(primaryDetumbling, experimentalAdcs, experimentalAdcs)
{
}

OSResult ObcAdcs::Initialize()
{
    const auto result = this->experimentalAdcs.Initialize();
    if (OS_RESULT_FAILED(result))
    {
        LOGF(LOG_LEVEL_ERROR, "[adcs] Unable to initialize experimental adcs. Reason: '%d'.", static_cast<int>(result));
    }

    return result;
}
