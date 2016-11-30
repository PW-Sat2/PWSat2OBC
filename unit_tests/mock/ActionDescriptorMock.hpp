#ifndef UNIT_TEST_ACTION_DESCRIPTOR_MOCK_HPP
#define UNIT_TEST_ACTION_DESCRIPTOR_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "state/state.h"

struct ActionDescriptorMock
{
    MOCK_METHOD1(Condition, bool(const SystemState* state));
    MOCK_METHOD1(Action, void(const SystemState* state));
    SystemActionDescriptor GetDescriptor();
};

#endif
