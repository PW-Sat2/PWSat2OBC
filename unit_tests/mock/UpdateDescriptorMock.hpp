#ifndef UNIT_TEST_UPDATE_DESCRIPTOR_MOCK_HPP
#define UNIT_TEST_UPDATE_DESCRIPTOR_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "state/state.h"

struct UpdateDescriptorMock
{
    MOCK_METHOD1(Update, SystemStateUpdateResult(SystemState* state));
    SystemStateUpdateDescriptor GetDescriptor();
};

#endif
