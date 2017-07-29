#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/comm.hpp"
#include "mock/error_counter.hpp"
#include "obc/telecommands/fdir.hpp"

using telecommunication::downlink::DownlinkAPID;
using testing::Return;

namespace
{
    class GetErrorCountersConfigTelecommandTest : public testing::Test
    {
      protected:
        template <typename... T> void Run(T... params);

        testing::NiceMock<TransmitterMock> _transmitter;

        ErrorCountingConfigrationMock _config;
        ErrorCountingMock _errors;

        obc::telecommands::GetErrorCountersConfigTelecommand _telecommand{_errors, _config};
    };

    template <typename... T> void GetErrorCountersConfigTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(_transmitter, buffer);
    }

    TEST_F(GetErrorCountersConfigTelecommandTest, ShouldRespondWithErrorCountersConfig)
    {
        // clang-format off
        std::array<std::uint8_t, 48> expectedPayload = {
				10, 101, 1, 4,
				11, 102, 2, 3,
				12, 103, 3, 2,
				13, 104, 4, 1,
				14, 105, 1, 4,
				15, 106, 2, 3,
				16, 107, 3, 2,
				17, 108, 4, 1,
				18, 109, 1, 4,
				19, 110, 2, 3,
				20, 111, 3, 2,
				21, 112, 4, 1
        };
        // clang-format on

        for (auto i = 0; i < 12; i++)
        {
            ON_CALL(_errors, Current(i)).WillByDefault(Return(10 + i));
            ON_CALL(_config, Limit(i)).WillByDefault(Return(101 + i));
            ON_CALL(_config, Increment(i)).WillByDefault(Return((i % 4) + 1));
            ON_CALL(_config, Decrement(i)).WillByDefault(Return(4 - (i % 4)));
        }

        EXPECT_CALL(_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::ErrorCounters, 0, expectedPayload)));

        Run();
    }
}
