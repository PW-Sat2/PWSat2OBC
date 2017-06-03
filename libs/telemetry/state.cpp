#include "telemetry/state.hpp"
#include "base/os.h"

namespace telemetry
{
    bool TelemetryState::Initialize()
    {
        this->bufferLock = System::CreateBinarySemaphore(0x20);
        return this->bufferLock != nullptr;
    }
}
