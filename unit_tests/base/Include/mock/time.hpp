#ifndef UNIT_TESTS_MOCK_TIME_HPP_
#define UNIT_TESTS_MOCK_TIME_HPP_

#include <chrono>
#include "gmock/gmock.h"
#include "mission/time.hpp"
#include "time/ICurrentTime.hpp"

struct CurrentTimeMock : public services::time::ICurrentTime
{
    MOCK_METHOD0(GetCurrentTime, Option<std::chrono::milliseconds>());
    MOCK_METHOD1(SetCurrentTime, bool(TimePoint));
    MOCK_METHOD1(SetCurrentTime, bool(std::chrono::milliseconds));
};

struct TimeSynchronizationMock : public mission::ITimeSynchronization
{
    MOCK_METHOD1(Lock, bool(std::chrono::milliseconds));
    MOCK_METHOD0(Unlock, void());
};

#endif /* UNIT_TESTS_MOCK_TIME_HPP_ */
