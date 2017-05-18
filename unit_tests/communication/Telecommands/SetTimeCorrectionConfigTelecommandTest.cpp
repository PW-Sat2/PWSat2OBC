#include <algorithm>
#include <array>
#include <cmath>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "base/reader.h"
#include "base/writer.h"
#include "mock/HasStateMock.hpp"
#include "mock/comm.hpp"
#include "mock/time.hpp"
#include "obc/telecommands/time.hpp"
#include "telecommunication/downlink.h"
#include "telecommunication/telecommand_handling.h"
#include "utils.hpp"

using std::array;
using std::uint8_t;
using testing::Eq;
using testing::ReturnRef;

using telecommunication::downlink::DownlinkFrame;
using telecommunication::downlink::DownlinkAPID;

namespace
{
    template <std::size_t Size> using Buffer = std::array<uint8_t, Size>;

    class SetTimeCorrectionConfigTelecommandTest : public testing::Test
    {
      protected:
        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<HasStateMock<SystemState>> _stateContainer;

        obc::telecommands::SetTimeCorrectionConfigTelecommand _telecommand{_stateContainer};
    };

    TEST_F(SetTimeCorrectionConfigTelecommandTest, ShouldSetState)
    {
        SystemState state;
        auto& persistentState = state.PersistentState;
        persistentState.Set(state::TimeCorrectionConfiguration(0x1111, 0x2222));
        ON_CALL(_stateContainer, MockGetState()).WillByDefault(ReturnRef(state));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteWordLE(0x1234);
        w.WriteWordLE(0x5678);

        _telecommand.Handle(_transmitter, w.Capture());

        auto& config = persistentState.Get<state::TimeCorrectionConfiguration>();
        ASSERT_THAT(config.MissionTimeFactor(), Eq(0x1234));
        ASSERT_THAT(config.ExternalTimeFactor(), Eq(0x5678));
    }

    TEST_F(SetTimeCorrectionConfigTelecommandTest, ShouldDenyZeroWeights)
    {
        SystemState state;
        auto& persistentState = state.PersistentState;
        persistentState.Set(state::TimeCorrectionConfiguration(0x1111, 0x2222));
        ON_CALL(_stateContainer, MockGetState()).WillByDefault(ReturnRef(state));

        Buffer<200> buffer;
        Writer w(buffer);
        w.WriteByte(0xFF);
        w.WriteWordLE(0x0);
        w.WriteWordLE(0x0);

        _telecommand.Handle(_transmitter, w.Capture());

        auto& config = persistentState.Get<state::TimeCorrectionConfiguration>();
        ASSERT_THAT(config.MissionTimeFactor(), Eq(0x1111));
        ASSERT_THAT(config.ExternalTimeFactor(), Eq(0x2222));
    }
}
