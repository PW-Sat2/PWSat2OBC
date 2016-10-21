#ifndef UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_
#define UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_

#include <functional>
#include <utility>
#include "rapidcheck.hpp"
#include "state/state.h"
#include "system.h"

class StateUpdater
{
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

  private:
    const char* name;
    std::function<SystemStateUpdateResult(SystemState*)> func;

    static SystemStateUpdateResult UpdateByMock(SystemState* state, void* param)
    {
        return ((StateUpdater*)param)->func(state);
    }
};

class StateVerifier
{
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

  private:
    const char* name;
    std::function<void(const SystemState*, SystemStateVerifyDescriptorResult*)> func;

    static void VerifyByMock(const SystemState* state, void* param, SystemStateVerifyDescriptorResult* result)
    {
        ((StateVerifier*)param)->func(state, result);
    }
};

class SystemAction
{
  public:
    SystemAction(const char* name) : name(name), executed(false)
    {
        actionDescriptor.Name = name;
        actionDescriptor.Condition = SystemAction::Condition;
        actionDescriptor.ActionProc = SystemAction::Action;
        actionDescriptor.Param = this;
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
};

namespace rc
{
    template <> struct Arbitrary<TimePoint>
    {
        static Gen<TimePoint> arbitrary()
        {
            auto day = gen::nonNegative<uint16_t>();
            auto hour = gen::inRange(0, 23);
            auto minute = gen::inRange(0, 59);
            auto second = gen::inRange(0, 59);
            auto milisecond = gen::inRange(0, 999);

            return gen::apply(
                [](uint16_t day, uint16_t hour, uint16_t minute, uint16_t second, uint16_t milisecond) {
                    return TimePointBuild(day, hour, minute, second, milisecond);
                },
                day,
                hour,
                minute,
                second,
                milisecond);
        }
    };

    template <> struct Arbitrary<SystemState>
    {
        static Gen<SystemState> arbitrary()
        {
            return gen::build<SystemState>(                                //
                gen::set(&SystemState::Time, gen::arbitrary<TimePoint>()), //
                gen::set(&SystemState::SailOpened, gen::arbitrary<bool>()) //
                );
        }
    };
}

void showValue(const SystemState& state, std::ostream& os);

#endif /* UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_ */
