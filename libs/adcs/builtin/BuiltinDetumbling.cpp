#include "BuiltinDetumbling.hpp"

namespace adcs
{
    BuiltinDetumbling::BuiltinDetumbling(devices::imtq::IImtqDriver& imtqDriver_) : imtqDriver(imtqDriver_)
    {
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
        imtqDriver.StartBDotDetumbling(std::chrono::seconds(1));
    }

    std::chrono::hertz BuiltinDetumbling::GetFrequency() const
    {
        return std::chrono::hertz(1);
    }
}
