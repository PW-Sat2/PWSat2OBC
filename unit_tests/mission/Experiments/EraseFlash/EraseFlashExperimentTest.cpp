#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "experiment/flash/flash.hpp"
#include "mock/comm.hpp"
#include "mock/error_counter.hpp"
#include "mock/n25q.hpp"

using testing::Eq;
using testing::Return;
using experiments::StartResult;
using testing::ByMove;
using testing::_;
using testing::ElementsAre;
using namespace devices::n25q;
using namespace std::chrono_literals;
using telecommunication::downlink::DownlinkAPID;

extern "C" void yaffsfs_Lock(void)
{
}

extern "C" void yaffsfs_Unlock(void)
{
}

namespace
{
    class EraseFlashExperimentTest : public testing::Test
    {
      protected:
        testing::NiceMock<ErrorCountingMock> _errors;
        testing::NiceMock<N25QDriverMock> _flash1;
        testing::NiceMock<N25QDriverMock> _flash2;
        testing::NiceMock<N25QDriverMock> _flash3;
        std::array<IN25QDriver*, 3> _flashes{{&_flash1, &_flash2, &_flash3}};

        RedundantN25QDriver _n25q{_errors, _flashes};

        testing::NiceMock<TransmitterMock> _transmitter;

        experiment::erase_flash::EraseFlashExperiment _exp{_n25q, _transmitter};
    };

    TEST_F(EraseFlashExperimentTest, StartReturnsSuccess)
    {
        ASSERT_THAT(_exp.Start(), Eq(StartResult::Success));
    }

    TEST_F(EraseFlashExperimentTest, ShouldSendSuccessFrame)
    {
        EXPECT_CALL(_flash1, BeginEraseChip()).WillOnce(Return(ByMove(OperationWaiter(&_flash1, 10ms, FlagStatus::Clear))));
        EXPECT_CALL(_flash2, BeginEraseChip()).WillOnce(Return(ByMove(OperationWaiter(&_flash2, 10ms, FlagStatus::Clear))));
        EXPECT_CALL(_flash3, BeginEraseChip()).WillOnce(Return(ByMove(OperationWaiter(&_flash3, 10ms, FlagStatus::Clear))));

        EXPECT_CALL(_flash1, WaitForOperation(_, _)).WillOnce(Return(OperationResult::Success));
        EXPECT_CALL(_flash2, WaitForOperation(_, _)).WillOnce(Return(OperationResult::Success));
        EXPECT_CALL(_flash3, WaitForOperation(_, _)).WillOnce(Return(OperationResult::Success));

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x68, 0, 2))));

        _exp.SetCorrelationId(0x68);

        _exp.Iteration();
    }

    TEST_F(EraseFlashExperimentTest, ShouldSendErrorFrameOnChipEraseFailure)
    {
        EXPECT_CALL(_flash1, BeginEraseChip()).WillOnce(Return(ByMove(OperationWaiter(&_flash1, 10ms, FlagStatus::Clear))));
        EXPECT_CALL(_flash2, BeginEraseChip()).WillOnce(Return(ByMove(OperationWaiter(&_flash2, 10ms, FlagStatus::Clear))));
        EXPECT_CALL(_flash3, BeginEraseChip()).WillOnce(Return(ByMove(OperationWaiter(&_flash3, 10ms, FlagStatus::Clear))));

        EXPECT_CALL(_flash1, WaitForOperation(_, _)).WillOnce(Return(OperationResult::Failure));
        EXPECT_CALL(_flash2, WaitForOperation(_, _)).WillOnce(Return(OperationResult::Failure));
        EXPECT_CALL(_flash3, WaitForOperation(_, _)).WillOnce(Return(OperationResult::Failure));

        EXPECT_CALL(
            _transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x68, 3, num(OperationResult::Failure)))));

        _exp.SetCorrelationId(0x68);

        _exp.Iteration();
    }
}
