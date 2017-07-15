#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/comm.hpp"
#include "obc/telecommands/fdir.hpp"

using testing::ElementsAre;
using testing::_;
using telecommunication::downlink::DownlinkAPID;

struct SetErrorCounterConfigMock : obc::ISetErrorCounterConfig
{
    MOCK_METHOD4(Set,
        void(error_counter::Device device,
            error_counter::CounterValue limit,
            error_counter::CounterValue increment,
            error_counter::CounterValue decrement));
};

namespace
{
    class SetErrorCounterConfigTelecommandTest : public testing::Test
    {
      protected:
        template <typename... T> void Run(T... params);

        testing::NiceMock<TransmitterMock> _transmitter;
        testing::StrictMock<SetErrorCounterConfigMock> _config;

        obc::telecommands::SetErrorCounterConfig _telecommand{_config};
    };

    template <typename... T> void SetErrorCounterConfigTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(_transmitter, buffer);
    }

    TEST_F(SetErrorCounterConfigTelecommandTest, ShouldSetErrorCountersConfig)
    {
        EXPECT_CALL(this->_config, Set(5, 137, 7, 2));
        EXPECT_CALL(this->_config, Set(9, 250, 0, 0));

        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x11, 0, 5, 9))));

        Run(0x11, 5, 137, 7, 2, 9, 250, 0, 0);
    }

    TEST_F(SetErrorCounterConfigTelecommandTest, ShouldRespondWithErrorFrameOnNoCorrelationId)
    {
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(_, 1))));

        Run();
    }

    TEST_F(SetErrorCounterConfigTelecommandTest, ShouldIgnoreLastEntryIfMalformed)
    {
        EXPECT_CALL(this->_config, Set(5, 137, 7, 2));

        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x11, 0, 5))));

        Run(0x11, 5, 137, 7, 2, 9, 250, 0);
    }

    TEST_F(SetErrorCounterConfigTelecommandTest, ShouldIgnoreInvalidDeviceId)
    {
        EXPECT_CALL(this->_config, Set(5, 137, 7, 2));

        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x11, 0, 5))));

        Run(0x11, 5, 137, 7, 2, 90, 250, 0, 0, 7, 137, 7, 2);
    }
}
