#ifndef UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_
#define UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_

#include <functional>
#include <utility>
#include "gmock/gmock.h"
#include "gmock-extensions.h"
#include "rapidcheck.h"
#include "state/state.h"
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
    std::function<void(const SystemState*, SystemStateVerifyDescriptorResult*)> func;

    static void VerifyByMock(const SystemState* state, void* param, SystemStateVerifyDescriptorResult* result)
    {
        ((StateVerifier*)param)->func(state, result);
    }

  public:
    StateVerifier(const char* name, std::function<void(const SystemState*, SystemStateVerifyDescriptorResult*)> f) : name(name), func(f)
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
    bool executed;

    std::function<bool(const SystemState*)> condition;
    std::function<void(const SystemState*)> action;

    SystemActionDescriptor actionDescriptor;

    static bool Condition(const SystemState* state, void* param)
    {
        return ((SystemAction*)param)->condition(state);
    }

    static void Action(const SystemState* state, void* param)
    {
        ((SystemAction*)param)->Execute(state);
    }

    void Execute(const SystemState* state)
    {
        this->executed = true;
        this->action(state);
    }

  public:
    SystemAction(const char* name) : name(name), executed(false)
    {
        actionDescriptor.Name = name;
        actionDescriptor.Condition = SystemAction::Condition;
        actionDescriptor.ActionProc = SystemAction::Action;
        actionDescriptor.Param = this;
        actionDescriptor.LastRun.Executed = false;
    }

    SystemAction() : SystemAction(nullptr)
    {
    }

    SystemAction& When(std::function<bool(const SystemState*)> condition)
    {
        this->condition = condition;

        return *this;
    }

    SystemAction& Do(std::function<void(const SystemState*)> action)
    {
        this->action = action;

        return *this;
    }

    SystemAction& Always()
    {
        this->When([](const SystemState* state) {
            UNREFERENCED_PARAMETER(state);
            return true;
        });

        return *this;
    }

    SystemAction& Never()
    {
        this->When([](const SystemState* state) {
            UNREFERENCED_PARAMETER(state);
            return false;
        });

        return *this;
    }

    SystemAction& DoNothing()
    {
        this->Do([](const SystemState* state) { UNREFERENCED_PARAMETER(state); });

        return *this;
    }

    operator SystemActionDescriptor*()
    {
        return &actionDescriptor;
    }

    operator SystemActionDescriptor()
    {
        return actionDescriptor;
    }

    bool WasExecuted()
    {
        return this->executed;
    }

    void ClearExecuted()
    {
        this->executed = false;
    }
};

namespace rc
{
    template <> struct Arbitrary<SystemState>
    {
        static Gen<SystemState> arbitrary()
        {
            return gen::build<SystemState>(                                 //
                gen::set(&SystemState::Time, gen::nonNegative<uint32_t>()), //
                gen::set(&SystemState::SailOpened, gen::arbitrary<bool>())  //
                );
        }
    };
}

void showValue(const SystemState& state, std::ostream& os);

#endif /* UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_ */
