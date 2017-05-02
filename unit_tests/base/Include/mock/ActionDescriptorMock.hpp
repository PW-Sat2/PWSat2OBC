#ifndef UNIT_TEST_ACTION_DESCRIPTOR_MOCK_HPP
#define UNIT_TEST_ACTION_DESCRIPTOR_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "mission/base.hpp"
#include "state/struct.h"

template <typename State, typename Tag> struct ActionDescriptorMock : public mission::Action
{
    MOCK_METHOD1_T(ConditionProc, bool(const State& state));
    MOCK_METHOD1_T(ActionProc, void(State& state));
    mission::ActionDescriptor<State> BuildAction();

    static bool ConditionEntry(const State& state, void* param);

    static void ActionEntry(State& state, void* param);
};

template <typename State, typename Tag>
bool ActionDescriptorMock<State, Tag>::ConditionEntry(const State& state, //
    void* param                                                           //
    )
{
    return static_cast<ActionDescriptorMock<State, Tag>*>(param)->ConditionProc(state);
}

template <typename State, typename Tag>
void ActionDescriptorMock<State, Tag>::ActionEntry(State& state, //
    void* param                                                  //
    )
{
    static_cast<ActionDescriptorMock<State, Tag>*>(param)->ActionProc(state);
}

template <typename State, typename Tag> mission::ActionDescriptor<State> ActionDescriptorMock<State, Tag>::BuildAction()
{
    mission::ActionDescriptor<State> descriptor;
    descriptor.actionProc = ActionEntry;
    descriptor.condition = ConditionEntry;
    descriptor.name = "Mock";
    descriptor.param = this;
    return descriptor;
}

#endif
