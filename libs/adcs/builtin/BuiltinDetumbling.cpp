#include "BuiltinDetumbling.hpp"
#include <chrono>

namespace adcs
{
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
        imtqDriver.StartBDotDetumbling(chrono_extensions::period_cast<std::chrono::seconds>(BuiltinDetumbling::Frequency));
    }

    std::chrono::milliseconds BuiltinDetumbling::GetWait() const
    {
        return chrono_extensions::period_cast<std::chrono::milliseconds>(BuiltinDetumbling::Frequency);
    }
}
