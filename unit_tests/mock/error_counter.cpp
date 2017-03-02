#include "error_counter.hpp"

using testing::_;
using testing::Return;

ErrorCountingConfigrationMock::ErrorCountingConfigrationMock()
{
    ON_CALL(*this, Increment(_)).WillByDefault(Return(1));
    ON_CALL(*this, Limit(_)).WillByDefault(Return(10));
}
