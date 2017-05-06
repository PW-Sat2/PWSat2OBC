#ifndef UNIT_TESTS_TIME_TIMESPAN_HPP_
#define UNIT_TESTS_TIME_TIMESPAN_HPP_

#include "time/TimePoint.h"

inline bool operator==(const TimePoint& left, const TimePoint& right)
{
    return TimePointEqual(left, right);
}

inline bool operator<(const TimePoint& left, const TimePoint& right)
{
    return TimePointLessThan(left, right);
}

#endif /* UNIT_TESTS_TIME_TIMESPAN_HPP_ */
