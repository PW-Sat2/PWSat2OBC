#include "BuiltinDetumbling.hpp"
#include <chrono>
#include "logger/logger.h"

using namespace std::chrono_literals;

namespace adcs
{
    constexpr std::chrono::milliseconds BuiltinDetumbling::GetWaitPeriod()
    {
        return chrono_extensions::period_cast<std::chrono::milliseconds>(BuiltinDetumbling::Frequency);
    }

    constexpr std::chrono::seconds BuiltinDetumbling::GetDetumblingPeriod()
    {
        return chrono_extensions::period_cast<std::chrono::seconds>(BuiltinDetumbling::Frequency) * 2;
    }

    BuiltinDetumbling::BuiltinDetumbling(devices::imtq::IImtqDriver& imtqDriver_) : imtqDriver(imtqDriver_)
    {
    }

    OSResult BuiltinDetumbling::Initialize()
    {
        return OSResult::Success;
    }

    OSResult BuiltinDetumbling::Enable()
    {
        return OSResult::Success;
    }

    OSResult BuiltinDetumbling::Disable()
    {
        return imtqDriver.CancelOperation() ? OSResult::Success : OSResult::IOError;
    }

    void BuiltinDetumbling::Process()
    {
        if (!imtqDriver.StartBDotDetumbling(GetDetumblingPeriod()))
        {
            LOG(LOG_LEVEL_ERROR, "Unable to run built-in detumbling");
            return;
        }

        System::SleepTask(2s);

        devices::imtq::DetumbleData result;
        if (!imtqDriver.GetDetumbleData(result))
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire built-in detumble data");
        }

        devices::imtq::HouseKeepingEngineering housekeeping;
        if (!imtqDriver.GetHouseKeepingEngineering(housekeeping))
        {
            LOG(LOG_LEVEL_ERROR, "Unable to acquire imtq housekeeping");
        }
    }

    std::chrono::milliseconds BuiltinDetumbling::GetWait() const
    {
        return GetWaitPeriod();
    }
}
