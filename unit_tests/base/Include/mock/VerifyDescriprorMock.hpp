#ifndef UNIT_TEST_VERIFY_DESCRIPTOR_MOCK_HPP
#define UNIT_TEST_VERIFY_DESCRIPTOR_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "mission/base.hpp"
#include "state/struct.h"

template <typename State, typename Tag> struct VerifyDescriptorMock : public mission::Verify
{
    MOCK_METHOD1_T(VerifyProc, mission::VerifyDescriptorResult(const State& state));
    mission::VerifyDescriptor<State> BuildVerify();

    static mission::VerifyDescriptorResult VerifyEntry(const State& state, void* param);
};

template <typename State, typename Tag>
mission::VerifyDescriptorResult VerifyDescriptorMock<State, Tag>::VerifyEntry(const State& state, void* param)
{
    return static_cast<VerifyDescriptorMock<State, Tag>*>(param)->VerifyProc(state);
}

template <typename State, typename Tag> mission::VerifyDescriptor<State> VerifyDescriptorMock<State, Tag>::BuildVerify()
{
    mission::VerifyDescriptor<State> descriptor;
    descriptor.name = "Mock";
    descriptor.param = this;
    descriptor.verifyProc = VerifyEntry;
    return descriptor;
}

#endif
