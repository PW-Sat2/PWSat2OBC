#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/comm.hpp"
#include "mock/power.hpp"

#include "obc/telecommands/power.hpp"

using testing::Eq;
using testing::Invoke;
using testing::_;
using testing::ElementsAre;
using telecommunication::downlink::DownlinkAPID;

namespace
{
    class PowerCycleTelecommandTest : public testing::Test
    {
      protected:
        template <typename... T> void Run(T... params);

        testing::NiceMock<TransmitterMock> _transmitter;
        PowerControlMock _power;

        obc::telecommands::PowerCycle _telecommand{_power};
    };

    template <typename... T> void PowerCycleTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        _telecommand.Handle(_transmitter, buffer);
    }

    TEST_F(PowerCycleTelecommandTest, ShouldPerformPowerCycle)
    {
        EXPECT_CALL(this->_power, PowerCycle());
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x11, 0x0))));
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x11, 0x2))));

        Run(0x11);
    }

    TEST_F(PowerCycleTelecommandTest, ShouldRespondWithErrorFrameOnInvalidFrame)
    {
        EXPECT_CALL(this->_power, PowerCycle()).Times(0);
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x0, 0x1))));

        Run();
    }
}
