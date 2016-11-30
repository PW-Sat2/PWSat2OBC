#include "ActionDescriptorMock.hpp"

static bool Condition(const SystemState* state, void* param)
{
    return static_cast<ActionDescriptorMock*>(param)->Condition(state);
}

static void Action(const SystemState* state, void* param)
{
    static_cast<ActionDescriptorMock*>(param)->Action(state);
}

SystemActionDescriptor ActionDescriptorMock::GetDescriptor()
{
    SystemActionDescriptor descriptor;
    descriptor.ActionProc = ::Action;
    descriptor.Condition = ::Condition;
    descriptor.Name = "Mock";
    descriptor.Param = this;
    return descriptor;
}
