#ifndef UNIT_TESTS_MOCK_TIME_HPP_
#define UNIT_TESTS_MOCK_TIME_HPP_

#include "gmock/gmock.h"
#include "time/ICurrentTime.hpp"

class CurrentTimeMock : public services::time::ICurrentTime
{
  public:
    MOCK_METHOD0(GetCurrentTime, Option<std::chrono::milliseconds>());

    MOCK_METHOD1(SetCurrentTime, bool(std::chrono::milliseconds));

    CurrentTimeMock();

  private:
    std::chrono::milliseconds _fakeTime;
};

#endif /* UNIT_TESTS_MOCK_TIME_HPP_ */
