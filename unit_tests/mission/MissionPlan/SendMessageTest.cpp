#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "mission/comm.hpp"
#include "mock/comm.hpp"
#include "telecommunication/downlink.h"

using namespace std::chrono_literals;
using testing::Eq;
using testing::ElementsAre;
using testing::_;
using testing::Return;
using telecommunication::downlink::DownlinkAPID;

namespace
{
    class SendMessageTest : public testing::Test
    {
      protected:
        SendMessageTest();

        constexpr static const char* Message = "Test";

        testing::NiceMock<OSMock> _os;
        OSReset _osReset{InstallProxy(&_os)};

        SystemState _state;
        testing::StrictMock<TransmitterMock> _transmitter;

        mission::SendMessageTask _task{_transmitter};
        mission::ActionDescriptor<SystemState> _action{_task.BuildAction()};
    };

    SendMessageTest::SendMessageTest()
    {
        ON_CALL(_os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));

        auto msgSpan = gsl::make_span(reinterpret_cast<const std::uint8_t*>(Message), 5);
        _state.PersistentState.Set(state::MessageState(5min, 3, msgSpan));
    }

    TEST_F(SendMessageTest, ShouldSendMessage)
    {
        _state.Time = 45min;
        _state.AntennaState.SetDeployment(true);

        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(true));

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::PeriodicMessage, 0, _))).Times(3);

        _action.Execute(_state);
    }

    TEST_F(SendMessageTest, ShouldNotSendMessageTooSoon)
    {
        _state.Time = 45min;
        _state.AntennaState.SetDeployment(false);

        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(false));
    }

    TEST_F(SendMessageTest, ShouldSendMessageAtIntervals)
    {
        _state.Time = 45min;
        _state.AntennaState.SetDeployment(true);

        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(true));

        EXPECT_CALL(_transmitter, SendFrame(_)).Times(3);
        _action.Execute(_state);

        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(false));

        _state.Time += 2min;

        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(false));

        _state.Time += 5min;

        ASSERT_THAT(_action.EvaluateCondition(_state), Eq(true));
    }
}
