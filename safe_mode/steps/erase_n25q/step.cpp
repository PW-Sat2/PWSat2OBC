#include "step.hpp"
#include "logger/logger.h"
#include "safe_mode.hpp"

void EraseN25QStep::Perform()
{
    LOG(LOG_LEVEL_INFO, "Erasing N25Q flashes");

    auto wait1 = SafeMode.Flash1.BeginEraseChip();
    auto wait2 = SafeMode.Flash2.BeginEraseChip();
    auto wait3 = SafeMode.Flash3.BeginEraseChip();

    LOG(LOG_LEVEL_INFO, "Flash1: Waiting for erase finish");
    auto result1 = wait1.Wait();
    LOGF(LOG_LEVEL_INFO, "Flash1 erase result: %d", num(result1));

    LOG(LOG_LEVEL_INFO, "Flash2: Waiting for erase finish");
    auto result2 = wait2.Wait();
    LOGF(LOG_LEVEL_INFO, "Flash2 erase result: %d", num(result2));

    LOG(LOG_LEVEL_INFO, "Flash3: Waiting for erase finish");
    auto result3 = wait3.Wait();
    LOGF(LOG_LEVEL_INFO, "Flash3 erase result: %d", num(result3));
}
