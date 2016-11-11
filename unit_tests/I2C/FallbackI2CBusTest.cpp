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

class FallbackI2CBusTest : public Test
{
  protected:
    testing::NiceMock<I2CBusMock> systemBus;
    testing::NiceMock<I2CBusMock> payloadBus;

    I2CInterface buses;

    I2CFallbackBus bus;

  public:
    FallbackI2CBusTest();
};

FallbackI2CBusTest::FallbackI2CBusTest()
{
    buses.Bus = &systemBus;
    buses.Payload = &payloadBus;

    I2CSetUpFallbackBus(&bus, &buses);
}

TEST_F(FallbackI2CBusTest, WriteReadShouldNotFallbackToPayloadBusIfSystemBusWorked)
{
    EXPECT_CALL(systemBus, I2CWriteRead(_, _, _, _, _, _)).WillOnce(Return(I2CResultOK));
    EXPECT_CALL(payloadBus, I2CWriteRead(_, _, _, _, _, _)).Times(0);

    uint8_t in[] = {1, 2, 3};
    uint8_t out[3] = {0};

    auto r = bus.Base.WriteRead((I2CBus*)&bus, 0x20, in, COUNT_OF(in), out, COUNT_OF(out));

    ASSERT_THAT(r, Eq(I2CResultOK));
}

TEST_F(FallbackI2CBusTest, WriteReadShouldFallbackToPayloadBusIfSystemBusFailed)
{
    EXPECT_CALL(systemBus, I2CWriteRead(_, _, _, _, _, _)).WillOnce(Return(I2CResultFailure));
    EXPECT_CALL(payloadBus, I2CWriteRead(_, _, _, _, _, _)).WillOnce(Return(I2CResultOK));

    uint8_t in[] = {1, 2, 3};
    uint8_t out[3] = {0};

    auto r = bus.Base.WriteRead((I2CBus*)&bus, 0x20, in, COUNT_OF(in), out, COUNT_OF(out));

    ASSERT_THAT(r, Eq(I2CResultOK));
}

TEST_F(FallbackI2CBusTest, WriteReadShouldFailWhenBothBusesFail)
{
    EXPECT_CALL(systemBus, I2CWriteRead(_, _, _, _, _, _)).WillOnce(Return(I2CResultFailure));
    EXPECT_CALL(payloadBus, I2CWriteRead(_, _, _, _, _, _)).WillOnce(Return(I2CResultNack));

    uint8_t in[] = {1, 2, 3};
    uint8_t out[3] = {0};

    auto r = bus.Base.WriteRead((I2CBus*)&bus, 0x20, in, COUNT_OF(in), out, COUNT_OF(out));

    ASSERT_THAT(r, Eq(I2CResultNack));
}

TEST_F(FallbackI2CBusTest, WriteShouldNotFallbackToPayloadBusIfSystemBusWorked)
{
    EXPECT_CALL(systemBus, I2CWrite(_, _, _, _)).WillOnce(Return(I2CResultOK));
    EXPECT_CALL(payloadBus, I2CWrite(_, _, _, _)).Times(0);

    uint8_t in[] = {1, 2, 3};

    auto r = bus.Base.Write((I2CBus*)&bus, 0x20, in, COUNT_OF(in));

    ASSERT_THAT(r, Eq(I2CResultOK));
}

TEST_F(FallbackI2CBusTest, WriteShouldFallbackToPayloadBusIfSystemBusFailed)
{
    EXPECT_CALL(systemBus, I2CWrite(_, _, _, _)).WillOnce(Return(I2CResultFailure));
    EXPECT_CALL(payloadBus, I2CWrite(_, _, _, _)).WillOnce(Return(I2CResultOK));

    uint8_t in[] = {1, 2, 3};

    auto r = bus.Base.Write((I2CBus*)&bus, 0x20, in, COUNT_OF(in));

    ASSERT_THAT(r, Eq(I2CResultOK));
}

TEST_F(FallbackI2CBusTest, WriteShouldFailWhenBothBusesFail)
{
    EXPECT_CALL(systemBus, I2CWrite(_, _, _, _)).WillOnce(Return(I2CResultFailure));
    EXPECT_CALL(payloadBus, I2CWrite(_, _, _, _)).WillOnce(Return(I2CResultNack));

    uint8_t in[] = {1, 2, 3};

    auto r = bus.Base.Write((I2CBus*)&bus, 0x20, in, COUNT_OF(in));

    ASSERT_THAT(r, Eq(I2CResultNack));
}
