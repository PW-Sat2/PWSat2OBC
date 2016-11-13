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
    StateUpdater(const char* name, std::function<SystemStateUpdateResult(SystemState*)> f);

    operator SystemStateUpdateDescriptor();

  private:
    static SystemStateUpdateResult UpdateByMock(SystemState* state, void* param);

    const char* name;
    std::function<SystemStateUpdateResult(SystemState*)> func;
};

class StateVerifier
{
  public:
    StateVerifier(const char* name,
        std::function<SystemStateVerifyDescriptorResult(const SystemState*)> f //
        );

    operator SystemStateVerifyDescriptor();

  private:
    static SystemStateVerifyDescriptorResult VerifyByMock(const SystemState* state, void* param);

    const char* name;

    std::function<SystemStateVerifyDescriptorResult(const SystemState*)> func;
};

class SystemAction
{
  public:
    SystemAction(const char* name);

    SystemAction();

    SystemAction& When(std::function<bool(const SystemState*)> condition);

    SystemAction& Do(std::function<void(const SystemState*)> action);

    SystemAction& Always();

    SystemAction& Never();

    SystemAction& DoNothing();

    operator SystemActionDescriptor*();

    operator SystemActionDescriptor();

    bool WasExecuted();

    void ClearExecuted();

  private:
    static bool Condition(const SystemState* state, void* param);

    static void Action(const SystemState* state, void* param);

    void Execute(const SystemState* state);

    const char* name;
    bool executed;

    std::function<bool(const SystemState*)> condition;
    std::function<void(const SystemState*)> action;

    SystemActionDescriptor actionDescriptor;
};

inline SystemAction::operator SystemActionDescriptor*()
{
    return &actionDescriptor;
}

inline SystemAction::operator SystemActionDescriptor()
{
    return actionDescriptor;
}

inline bool SystemAction::WasExecuted()
{
    return this->executed;
}

inline void SystemAction::ClearExecuted()
{
    this->executed = false;
}

namespace rc
{
    template <> struct Arbitrary<TimeSpan>
    {
        static Gen<TimeSpan> arbitrary()
        {
            auto day = gen::nonNegative<uint16_t>();
            auto hour = gen::inRange(0, 23);
            auto minute = gen::inRange(0, 59);
            auto second = gen::inRange(0, 59);
            auto milisecond = gen::inRange(0, 999);

            return gen::apply(
                [](uint16_t day, uint16_t hour, uint16_t minute, uint16_t second, uint16_t milisecond) {
                    return TimePointToTimeSpan(TimePointBuild(day, hour, minute, second, milisecond));
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
                gen::set(&SystemState::Time, gen::arbitrary<TimeSpan>()),  //
                gen::set(&SystemState::SailOpened, gen::arbitrary<bool>()) //
                );
        }
    };
}

void showValue(const SystemState& state, std::ostream& os);

#endif /* UNIT_TESTS_MISSIONPLAN_MISSIONTESTHELPERS_H_ */
