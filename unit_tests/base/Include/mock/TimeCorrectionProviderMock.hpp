#ifndef UNIT_TESTS_BASE_INCLUDE_MOCK_TIMECORRECTIONPROVIDERMOCK_HPP_
#define UNIT_TESTS_BASE_INCLUDE_MOCK_TIMECORRECTIONPROVIDERMOCK_HPP_

#include "gmock/gmock.h"
#include "state/time/ITimeCorrectionProvider.hpp"

class TimeCorrectionProviderMock : public state::ITimeCorrectionProvider
{
  public:
    MOCK_METHOD0(GetCurrentTimeCorrectionFactor, int16_t());

    MOCK_METHOD0(GetCurrentExternalTimeCorrectionFactor, int16_t());

    MOCK_METHOD1(SetCurrentTimeCorrectionFactor, bool(int16_t));

    MOCK_METHOD1(SetCurrentExternalTimeCorrectionFactor, bool(int16_t));

    TimeCorrectionProviderMock();

  private:
    int16_t _fakeTimeCorectionFactor;
    int16_t _fakeExternalTimeCorectionFactor;
};

#endif /* UNIT_TESTS_BASE_INCLUDE_MOCK_TIMECORRECTIONPROVIDERMOCK_HPP_ */
