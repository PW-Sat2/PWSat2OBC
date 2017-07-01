#include "InterruptPinDriverMock.hpp"

InterruptPinDriverMock::InterruptPinDriverMock() : _pin(0)
{
    ON_CALL(*this, Value()).WillByDefault(Invoke([this]() { return _pin; }));
}
