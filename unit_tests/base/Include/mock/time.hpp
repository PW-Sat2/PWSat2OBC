#ifndef UNIT_TESTS_MOCK_TIME_HPP_
#define UNIT_TESTS_MOCK_TIME_HPP_

#include "gmock/gmock.h"
#include "time/ICurrentTime.hpp"

struct CurrentTimeMock : public services::time::ICurrentTime
{
    MOCK_METHOD0(GetCurrentTime, Option<std::chrono::milliseconds>());
    MOCK_METHOD1(SetCurrentTime, bool(TimePoint));
    MOCK_METHOD1(SetCurrentTime, bool(std::chrono::milliseconds));
};

#endif /* UNIT_TESTS_MOCK_TIME_HPP_ */
