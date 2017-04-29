#include "time.hpp"
#include "utils.h"

using testing::Invoke;
using testing::_;

using namespace std::chrono;

CurrentTimeMock::CurrentTimeMock() : _fakeTime(0)
{
    ON_CALL(*this, GetCurrentTime()).WillByDefault(Invoke([this]() {
        if (_fakeTime == _fakeTime.zero())
        {
        	_fakeTime = milliseconds(static_cast<uint64_t>(time(0)));
        }

        return Some(_fakeTime);
    }));

    ON_CALL(*this, SetCurrentTime(_)).WillByDefault(Invoke([this](std::chrono::milliseconds newTime) {
        this->_fakeTime = newTime;
        return true;
    }));
}

