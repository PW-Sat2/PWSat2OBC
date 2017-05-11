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

    std::chrono::milliseconds BuiltinDetumbling::GetWait() const
    {
        return std::chrono::milliseconds{1};
    }
}
