#include "telemetry/state.hpp"
#include "base/os.h"

namespace telemetry
{
    bool TelemetryState::Initialize()
    {
        this->bufferLock = System::CreateBinarySemaphore(0x20);
        auto status = this->bufferLock != nullptr;
        if (status)
        {
            status = OS_RESULT_SUCCEEDED(System::GiveSemaphore(this->bufferLock));
        }

        return status;
    }
}
