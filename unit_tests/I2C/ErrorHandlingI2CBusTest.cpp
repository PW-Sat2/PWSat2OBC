#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"
#include "I2CMock.hpp"
#include "i2c/i2c.h"
#include "system.h"

using testing::Test;
using testing::_;
using testing::Return;
using testing::Eq;

class ErrorHandlingI2CBusTest : public Test
{
  protected:
    testing::NiceMock<I2CBusMock> innerBus;

    I2CErrorHandlingBus bus;

    static I2CResult HandlerProc(I2CBus* bus, I2CResult result, I2CAddress address, void* context)
    {
        return static_cast<ErrorHandlingI2CBusTest*>(context)->Handler(bus, result, address, context);
    }

    MOCK_METHOD4(Handler, I2CResult(I2CBus* bus, I2CResult result, I2CAddress address, void* context));

  public:
    ErrorHandlingI2CBusTest();
};

ErrorHandlingI2CBusTest::ErrorHandlingI2CBusTest() : bus(&innerBus, HandlerProc, this)
{
}

TEST_F(ErrorHandlingI2CBusTest, WriteReadShouldCallInnerBusAndPassthroughSuccess)
{
    EXPECT_CALL(innerBus, WriteRead(_, _, _)).WillOnce(Return(I2CResult::OK));
    EXPECT_CALL(*this, Handler(_, _, _, _)).Times(0);

    uint8_t in[] = {1, 2, 3};
    uint8_t out[3] = {0};

    auto r = bus.WriteRead(0x20, in, out);

    ASSERT_THAT(r, Eq(I2CResult::OK));
}

TEST_F(ErrorHandlingI2CBusTest, WriteReadShouldCallInnerBusAndExecuteHandlerOnError)
{
    EXPECT_CALL(innerBus, WriteRead(_, _, _)).WillOnce(Return(I2CResult::BusErr));
    EXPECT_CALL(*this, Handler(_, _, _, _)).WillOnce(Return(I2CResult::OK));

    uint8_t in[] = {1, 2, 3};
    uint8_t out[3] = {0};

    auto r = bus.WriteRead(0x20, in, out);

    ASSERT_THAT(r, Eq(I2CResult::OK));
}

TEST_F(ErrorHandlingI2CBusTest, WriteShouldCallInnerBusAndPassthroughSuccess)
{
    EXPECT_CALL(innerBus, Write(_, _)).WillOnce(Return(I2CResult::OK));
    EXPECT_CALL(*this, Handler(_, _, _, _)).Times(0);

    uint8_t in[] = {1, 2, 3};

    auto r = bus.Write(0x20, in);

    ASSERT_THAT(r, Eq(I2CResult::OK));
}

TEST_F(ErrorHandlingI2CBusTest, WriteShouldCallInnerBusAndExecuteHandlerOnError)
{
    EXPECT_CALL(innerBus, Write(_, _)).WillOnce(Return(I2CResult::BusErr));
    EXPECT_CALL(*this, Handler(_, _, _, _)).WillOnce(Return(I2CResult::OK));

    uint8_t in[] = {1, 2, 3};

    auto r = bus.Write(0x20, in);

    ASSERT_THAT(r, Eq(I2CResult::OK));
}
