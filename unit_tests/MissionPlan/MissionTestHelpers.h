#ifndef UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_
#define UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_

#include <functional>
#include "gmock/gmock.h"
#include "gmock-extensions.h"
#include "mission/mission.h"
#include "rapidcheck.h"
#include "system.h"

using testing::_;
using testing::AtLeast;

class StateUpdater
{
  private:
    const char* name;
    std::function<SystemStateUpdateResult(SystemState*)> func;

    static SystemStateUpdateResult UpdateByMock(SystemState* state, void* param)
    {
        return ((StateUpdater*)param)->func(state);
    }

  public:
    MOCK_METHOD2(UpdateState, SystemStateUpdateResult(SystemState* state, void* param));
    StateUpdater(const char* name, std::function<SystemStateUpdateResult(SystemState*)> f) : name(name), func(f)
    {
    }

    operator SystemStateUpdateDescriptor()
    {
        SystemStateUpdateDescriptor desc;

        desc.Param = this;
        desc.Name = name;
        desc.UpdateProc = StateUpdater::UpdateByMock;

        return desc;
    }
};

class StateVerifier
{
  private:
    const char* name;
    std::function<void(SystemState*, SystemStateVerifyDescriptorResult*)> func;

    static void VerifyByMock(SystemState* state, void* param, SystemStateVerifyDescriptorResult* result)
    {
        ((StateVerifier*)param)->func(state, result);
    }

  public:
    StateVerifier(const char* name, std::function<void(SystemState*, SystemStateVerifyDescriptorResult*)> f) : name(name), func(f)
    {
    }

    operator SystemStateVerifyDescriptor()
    {
        SystemStateVerifyDescriptor desc;

        desc.Param = this;
        desc.Name = name;
        desc.VerifyProc = StateVerifier::VerifyByMock;

        return desc;
    }
};

class SystemAction
{
  private:
    const char* name;
    std::function<bool(SystemState*)> condition;
    std::function<void(SystemState*)> action;

    SystemActionDescriptor actionDescriptor;

    static bool Condition(SystemState* state, void* param)
    {
        return ((SystemAction*)param)->condition(state);
    }

    static void Action(SystemState* state, void* param)
    {
        ((SystemAction*)param)->action(state);
    }

  public:
    SystemAction(const char* name) : name(name)
    {
        actionDescriptor.Name = name;
        actionDescriptor.Condition = SystemAction::Condition;
        actionDescriptor.ActionProc = SystemAction::Action;
        actionDescriptor.Param = this;
    }

    SystemAction& When(std::function<bool(SystemState*)> condition)
    {
        this->condition = condition;

        return *this;
    }

    SystemAction& Do(std::function<void(SystemState*)> action)
    {
        this->action = action;

        return *this;
    }

    operator SystemActionDescriptor*()
    {
        return &actionDescriptor;
    }
};

namespace rc
{
    template <> struct Arbitrary<SystemState>
    {
        static Gen<SystemState> arbitrary()
        {
            return gen::build<SystemState>(                                //
                gen::set(&SystemState::time, gen::nonNegative<uint32_t>()) //
                );
        }
    };
}

void showValue(const SystemState& state, std::ostream& os);

#endif /* UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_ */
