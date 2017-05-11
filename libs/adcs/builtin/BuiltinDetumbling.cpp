#include "BuiltinDetumbling.hpp"

namespace adcs
{
    OSResult BuiltinDetumbling::Enable()
    {
        return OSResult::Success;
    }

    OSResult BuiltinDetumbling::Disable()
    {
        return OSResult::Success;
    }

    void BuiltinDetumbling::Process()
    {
    }

    std::chrono::hertz BuiltinDetumbling::GetFrequency() const
    {
        return std::chrono::hertz(1);
    }
}
