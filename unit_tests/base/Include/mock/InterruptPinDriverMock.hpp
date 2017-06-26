#ifndef UNIT_TESTS_BASE_INCLUDE_MOCK_INTERRUPTPINDRIVERMOCK_HPP_
#define UNIT_TESTS_BASE_INCLUDE_MOCK_INTERRUPTPINDRIVERMOCK_HPP_

#include <gsl/span>
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "gpio/InterruptPinDriver.h"

using testing::Invoke;

class InterruptPinDriverMock : public drivers::gpio::IInterruptPinDriver
{
  public:
    MOCK_METHOD1(EnableInterrupt, void(const int32_t));

    MOCK_METHOD0(ClearInterrupt, void());

    MOCK_CONST_METHOD0(Value, bool());

    inline void SetValue(bool newValue)
    {
        _pin = newValue;
    }

    InterruptPinDriverMock() : _pin(0)
    {
        ON_CALL(*this, Value()).WillByDefault(Invoke([this]() { return _pin; }));
    }

  private:
    bool _pin;
};

#endif /* UNIT_TESTS_BASE_INCLUDE_MOCK_INTERRUPTPINDRIVERMOCK_HPP_ */
