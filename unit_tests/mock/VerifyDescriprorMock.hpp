#ifndef UNIT_TEST_VERIFY_DESCRIPTOR_MOCK_HPP
#define UNIT_TEST_VERIFY_DESCRIPTOR_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "state/state.h"

struct VerifyDescriptorMock
{
    MOCK_METHOD1(Verify, SystemStateVerifyDescriptorResult(const SystemState* state));
    SystemStateVerifyDescriptor GetDescriptor();
};

#endif
