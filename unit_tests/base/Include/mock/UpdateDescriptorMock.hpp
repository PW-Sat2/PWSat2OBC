#ifndef UNIT_TEST_UPDATE_DESCRIPTOR_MOCK_HPP
#define UNIT_TEST_UPDATE_DESCRIPTOR_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "mission/base.hpp"
#include "state/struct.h"

template <typename State, typename Tag> struct UpdateDescriptorMock : public mission::Update
{
    MOCK_METHOD1_T(UpdateProc, mission::UpdateResult(State& state));
    mission::UpdateDescriptor<State> BuildUpdate();
    static mission::UpdateResult UpdateEntry(State& state, void* param);
};

template <typename State, typename Tag> mission::UpdateResult UpdateDescriptorMock<State, Tag>::UpdateEntry(State& state, void* param)
{
    return static_cast<UpdateDescriptorMock<State, Tag>*>(param)->UpdateProc(state);
}

template <typename State, typename Tag> mission::UpdateDescriptor<State> UpdateDescriptorMock<State, Tag>::BuildUpdate()
{
    mission::UpdateDescriptor<State> descriptor;
    descriptor.name = "Mock";
    descriptor.updateProc = UpdateEntry;
    descriptor.param = this;
    return descriptor;
}

#endif
