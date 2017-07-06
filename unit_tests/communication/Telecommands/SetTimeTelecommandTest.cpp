#include <algorithm>
#include <array>
#include <cmath>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "OsMock.hpp"
#include "base/reader.h"
#include "base/writer.h"
#include "mock/HasStateMock.hpp"
#include "mock/RtcMock.hpp"
#include "mock/comm.hpp"
#include "mock/time.hpp"
#include "obc/telecommands/time.hpp"
#include "os/os.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"
#include "utils.hpp"

using std::array;
using std::uint8_t;
using testing::_;
using testing::Eq;
using testing::Return;
using testing::ReturnRef;

using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace
{
    template <std::size_t Size> using Buffer = std::array<uint8_t, Size>;

    class SetTimeTelecommandTest : public testing::Test
    {
      protected:
        SetTimeTelecommandTest();
        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<HasStateMock<SystemState>> _stateContainer;
        testing::NiceMock<CurrentTimeMock> _currentTime;
        testing::NiceMock<TimeSynchronizationMock> _timeSynchronization;

        RtcMock _rtc;

        testing::NiceMock<OSMock> os;
        OSReset guard;

        obc::telecommands::SetTimeTelecommand _telecommand{_stateContainer, _currentTime, _rtc, _timeSynchronization};
    };

    SetTimeTelecommandTest::SetTimeTelecommandTest()
    {
        this->guard = InstallProxy(&os);
        ON_CALL(os, TakeSemaphore(_, _)).WillByDefault(Return(OSResult::Success));
        ON_CALL(_timeSynchronization, AcquireTimeSynchronizationLock()).WillByDefault(Return(true));
        ON_CALL(_timeSynchronization, ReleaseTimeSynchronizationLock()).WillByDefault(Return(true));
    }

    TEST_F(SetTimeTelecommandTest, ShouldSetState)
    {
        SystemState state;
        auto& persistentState = state.PersistentState;
        persistentState.Set(state::TimeState(std::chrono::seconds{0x11111111}, std::chrono::seconds{0x22222222}));
        ON_CALL(_stateContainer, MockGetState()).WillByDefault(ReturnRef(state));
        EXPECT_CALL(_currentTime, SetCurrentTime(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::seconds{0x87654321})))
            .WillOnce(Return(true));
        _rtc.SetReadResult(OSResult::Success);

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteDoubleWordLE(0x87654321);

        _telecommand.Handle(_transmitter, w.Capture());

        state::TimeState timeState;
        persistentState.Get(timeState);
        ASSERT_THAT(timeState.LastMissionTime(), Eq(std::chrono::seconds{0x87654321}));
    }
}
