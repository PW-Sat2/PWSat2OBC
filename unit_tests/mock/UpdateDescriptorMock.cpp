#include "UpdateDescriptorMock.hpp"

static SystemStateUpdateResult Update(SystemState* state, void* param)
{
    return static_cast<UpdateDescriptorMock*>(param)->Update(state);
}

SystemStateUpdateDescriptor UpdateDescriptorMock::GetDescriptor()
{
    SystemStateUpdateDescriptor descriptor;
    descriptor.Name = "Mock";
    descriptor.UpdateProc = ::Update;
    descriptor.Param = this;
    return descriptor;
}
