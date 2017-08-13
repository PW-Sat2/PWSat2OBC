#include <algorithm>
#include <array>
#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "I2C/I2CMock.hpp"
#include "OsMock.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "comm/CommDriver.hpp"
#include "comm/Frame.hpp"
#include "mock/error_counter.hpp"
#include "utils.hpp"

using testing::_;
using testing::Eq;
using testing::ElementsAre;
using testing::Invoke;
using testing::Return;
using testing::DoAll;
using testing::InSequence;

using gsl::span;

using namespace devices::comm;

namespace
{
    class CommThreadsafeTest : public testing::Test
    {
      protected:
        CommThreadsafeTest();

        testing::NiceMock<I2CBusMock> i2c;
        testing::NiceMock<ErrorCountingConfigrationMock> errorsConfig;

        testing::NiceMock<OSMock> os;
        OSReset guard;

        OSSemaphoreHandle transmitterSemaphore{reinterpret_cast<OSSemaphoreHandle>(0x1234)};
        OSSemaphoreHandle receiverSemaphore{reinterpret_cast<OSSemaphoreHandle>(0x5678)};
        error_counter::ErrorCounting errors{errorsConfig};
    };

    CommThreadsafeTest::CommThreadsafeTest()
    {
        this->guard = InstallProxy(&os);

        ON_CALL(os, CreateBinarySemaphore(CommObject::transmitterSemaphoreId)).WillByDefault(Return(transmitterSemaphore));
        ON_CALL(os, CreateBinarySemaphore(CommObject::receiverSemaphoreId)).WillByDefault(Return(receiverSemaphore));
    }

    TEST_F(CommThreadsafeTest, ShouldSynchronizeTransmitter)
    {
        {
            InSequence dummy;
            UNREFERENCED_PARAMETER(dummy);

            EXPECT_CALL(os, GiveSemaphore(transmitterSemaphore)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(os, TakeSemaphore(transmitterSemaphore, _)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(os, GiveSemaphore(transmitterSemaphore)).WillOnce(Return(OSResult::Success));
        }

        EXPECT_CALL(os, TakeSemaphore(receiverSemaphore, _)).Times(0);
        EXPECT_CALL(os, GiveSemaphore(receiverSemaphore)).WillOnce(Return(OSResult::Success));

        CommObject comm{errors, i2c};
        comm.Initialize();

        uint8_t buffer[] = {0x1};
        comm.SendFrame(span<const uint8_t>(buffer));
    }

    TEST_F(CommThreadsafeTest, ShouldSynchronizeReceiver)
    {
        {
            InSequence dummy;
            UNREFERENCED_PARAMETER(dummy);

            EXPECT_CALL(os, GiveSemaphore(receiverSemaphore)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(os, TakeSemaphore(receiverSemaphore, _)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(os, GiveSemaphore(receiverSemaphore)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(os, TakeSemaphore(receiverSemaphore, _)).WillOnce(Return(OSResult::Success));
            EXPECT_CALL(os, GiveSemaphore(receiverSemaphore)).WillOnce(Return(OSResult::Success));
        }

        EXPECT_CALL(os, TakeSemaphore(transmitterSemaphore, _)).Times(0);
        EXPECT_CALL(os, GiveSemaphore(transmitterSemaphore)).WillOnce(Return(OSResult::Success));

        CommObject comm{errors, i2c};
        comm.Initialize();

        uint8_t buffer[] = {0x1, 0x2};
        Frame frame;
        comm.ReceiveFrame(buffer, frame);
    }
}
