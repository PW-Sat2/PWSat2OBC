#include "TimeCorrectionProviderMock.hpp"
#include "utils.h"

using testing::Invoke;
using testing::_;

TimeCorrectionProviderMock::TimeCorrectionProviderMock() : _fakeTimeCorectionFactor(1), _fakeExternalTimeCorectionFactor(1)
{
    ON_CALL(*this, GetCurrentTimeCorrectionFactor()).WillByDefault(Invoke([this]() { return _fakeTimeCorectionFactor; }));

    ON_CALL(*this, GetCurrentExternalTimeCorrectionFactor()).WillByDefault(Invoke([this]() { return _fakeExternalTimeCorectionFactor; }));

    ON_CALL(*this, SetCurrentTimeCorrectionFactor(_)).WillByDefault(Invoke([this](int16_t newFactor) {
        this->_fakeTimeCorectionFactor = newFactor;
        return true;
    }));

    ON_CALL(*this, SetCurrentExternalTimeCorrectionFactor(_)).WillByDefault(Invoke([this](int16_t newFactor) {
        this->_fakeExternalTimeCorectionFactor = newFactor;
        return true;
    }));
}
