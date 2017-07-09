#include "step.hpp"

#include "logger/logger.h"
#include "safe_mode.hpp"

void RevertBootSlots::Perform()
{
    LOG(LOG_LEVEL_INFO, "Reverting boot slots to default");

    LOG(LOG_LEVEL_DEBUG, "Reseting boot settings");

    SafeMode.BootSettings.BootSlots(0b000111);
    SafeMode.BootSettings.FailsafeBootSlots(0b111000);
    SafeMode.BootSettings.BootCounter(0);
    SafeMode.BootSettings.ConfirmBoot();
    SafeMode.BootSettings.MarkAsValid();
}
