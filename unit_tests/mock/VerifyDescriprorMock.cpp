#include "VerifyDescriprorMock.hpp"

static SystemStateVerifyDescriptorResult Verify(const SystemState* state, void* param)
{
    return static_cast<VerifyDescriptorMock*>(param)->Verify(state);
}

SystemStateVerifyDescriptor VerifyDescriptorMock::GetDescriptor()
{
    SystemStateVerifyDescriptor descriptor;
    descriptor.Name = "Mock";
    descriptor.Param = this;
    descriptor.VerifyProc = ::Verify;
    return descriptor;
}
