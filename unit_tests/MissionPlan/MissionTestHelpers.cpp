#include "MissionTestHelpers.h"
#include <ostream>
#include "state/state.h"

StateUpdater::StateUpdater(const char* name, std::function<SystemStateUpdateResult(SystemState*)> f)
    : name(name), //
      func(f)
{
}

StateUpdater::operator SystemStateUpdateDescriptor()
{
    SystemStateUpdateDescriptor desc;

    desc.Param = this;
    desc.Name = name;
    desc.UpdateProc = StateUpdater::UpdateByMock;

    return desc;
}

SystemStateUpdateResult StateUpdater::UpdateByMock(SystemState* state, void* param)
{
    return ((StateUpdater*)param)->func(state);
}

StateVerifier::StateVerifier(const char* name,                             //
    std::function<SystemStateVerifyDescriptorResult(const SystemState*)> f //
    )
    : name(name), //
      func(f)
{
}

StateVerifier::operator SystemStateVerifyDescriptor()
{
    SystemStateVerifyDescriptor desc;

    desc.Param = this;
    desc.Name = name;
    desc.VerifyProc = StateVerifier::VerifyByMock;

    return desc;
}

SystemStateVerifyDescriptorResult StateVerifier::VerifyByMock(const SystemState* state, void* param)
{
    return ((StateVerifier*)param)->func(state);
}

SystemAction::SystemAction(const char* name) : name(name), executed(false)
{
    actionDescriptor.Name = name;
    actionDescriptor.Condition = SystemAction::Condition;
    actionDescriptor.ActionProc = SystemAction::Action;
    actionDescriptor.Param = this;
}

SystemAction::SystemAction() : SystemAction(nullptr)
{
}

SystemAction& SystemAction::When(std::function<bool(const SystemState*)> condition)
{
    this->condition = condition;

    return *this;
}

SystemAction& SystemAction::Do(std::function<void(const SystemState*)> action)
{
    this->action = action;

    return *this;
}

SystemAction& SystemAction::Always()
{
    this->When([](const SystemState* state) {
        UNREFERENCED_PARAMETER(state);
        return true;
    });

    return *this;
}

SystemAction& SystemAction::Never()
{
    this->When([](const SystemState* state) {
        UNREFERENCED_PARAMETER(state);
        return false;
    });

    return *this;
}

SystemAction& SystemAction::DoNothing()
{
    this->Do([](const SystemState* state) { UNREFERENCED_PARAMETER(state); });

    return *this;
}

bool SystemAction::Condition(const SystemState* state, void* param)
{
    return ((SystemAction*)param)->condition(state);
}

void SystemAction::Action(const SystemState* state, void* param)
{
    ((SystemAction*)param)->Execute(state);
}

void SystemAction::Execute(const SystemState* state)
{
    this->executed = true;
    this->action(state);
}

void showValue(const SystemState& state, std::ostream& os)
{
    os << "time=" << state.Time.value << ", "
       << "antennaDeployed=" << state.Antenna.Deployed;
}
