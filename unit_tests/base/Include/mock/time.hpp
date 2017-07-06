#ifndef UNIT_TESTS_MOCK_TIME_HPP_
#define UNIT_TESTS_MOCK_TIME_HPP_

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
    MOCK_METHOD0(AcquireTimeSynchronizationLock, bool());

    MOCK_METHOD0(ReleaseTimeSynchronizationLock, bool());
};

#endif /* UNIT_TESTS_MOCK_TIME_HPP_ */
