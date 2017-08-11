#ifndef MOCK_OPEN_SAIL_MOCK_HPP
#define MOCK_OPEN_SAIL_MOCK_HPP

#pragma once

#include "gmock/gmock.h"
#include "mission/sail.hpp"

struct OpenSailMock : mission::IOpenSail
{
    OpenSailMock();

    ~OpenSailMock();

    MOCK_METHOD0(OpenSail, void());
};

#endif
