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

ErrorHandlingI2CBusTest::ErrorHandlingI2CBusTest()
{
    I2CSetUpErrorHandlingBus(&bus, &innerBus, HandlerProc, this);
}

TEST_F(ErrorHandlingI2CBusTest, WriteReadShouldCallInnerBusAndPassthroughSuccess)
{
    EXPECT_CALL(innerBus, I2CWriteRead(_, _, _, _, _, _)).WillOnce(Return(I2CResultOK));
    EXPECT_CALL(*this, Handler(_, _, _, _)).Times(0);

    uint8_t in[] = {1, 2, 3};
    uint8_t out[3] = {0};

    auto r = bus.OuterBus.WriteRead(&bus.OuterBus, 0x20, in, COUNT_OF(in), out, COUNT_OF(out));

    ASSERT_THAT(r, Eq(I2CResultOK));
}

TEST_F(ErrorHandlingI2CBusTest, WriteReadShouldCallInnerBusAndExecuteHandlerOnError)
{
    EXPECT_CALL(innerBus, I2CWriteRead(_, _, _, _, _, _)).WillOnce(Return(I2CResultBusErr));
    EXPECT_CALL(*this, Handler(_, _, _, _)).WillOnce(Return(I2CResultOK));

    uint8_t in[] = {1, 2, 3};
    uint8_t out[3] = {0};

    auto r = bus.OuterBus.WriteRead(&bus.OuterBus, 0x20, in, COUNT_OF(in), out, COUNT_OF(out));

    ASSERT_THAT(r, Eq(I2CResultOK));
}

TEST_F(ErrorHandlingI2CBusTest, WriteShouldCallInnerBusAndPassthroughSuccess)
{
    EXPECT_CALL(innerBus, I2CWrite(_, _, _, _)).WillOnce(Return(I2CResultOK));
    EXPECT_CALL(*this, Handler(_, _, _, _)).Times(0);

    uint8_t in[] = {1, 2, 3};

    auto r = bus.OuterBus.Write(&bus.OuterBus, 0x20, in, COUNT_OF(in));

    ASSERT_THAT(r, Eq(I2CResultOK));
}

TEST_F(ErrorHandlingI2CBusTest, WriteShouldCallInnerBusAndExecuteHandlerOnError)
{
    EXPECT_CALL(innerBus, I2CWrite(_, _, _, _)).WillOnce(Return(I2CResultBusErr));
    EXPECT_CALL(*this, Handler(_, _, _, _)).WillOnce(Return(I2CResultOK));

    uint8_t in[] = {1, 2, 3};

    auto r = bus.OuterBus.Write(&bus.OuterBus, 0x20, in, COUNT_OF(in));

    ASSERT_THAT(r, Eq(I2CResultOK));
}
