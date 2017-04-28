#include "error_counter.hpp"

using testing::_;
using testing::Return;

ErrorCountingConfigrationMock::ErrorCountingConfigrationMock()
{
    ON_CALL(*this, Increment(_)).WillByDefault(Return(5));
    ON_CALL(*this, Decrement(_)).WillByDefault(Return(2));
    ON_CALL(*this, Limit(_)).WillByDefault(Return(50));
}
