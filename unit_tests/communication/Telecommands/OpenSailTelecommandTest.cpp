#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mission/sail.hpp"
#include "mock/OpenSailMock.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/sail.hpp"

using telecommunication::downlink::DownlinkAPID;
using testing::ElementsAre;

namespace
{
    class OpenSailTelecommandTest : public testing::Test
    {
      protected:
        template <typename... T> void Run(T... params);

        testing::NiceMock<TransmitterMock> _transmitter;
        testing::StrictMock<OpenSailMock> _openSail;

        obc::telecommands::OpenSail _telecommand{_openSail};
    };

    template <typename... T> void OpenSailTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        this->_telecommand.Handle(_transmitter, buffer);
    }

    TEST_F(OpenSailTelecommandTest, ShouldOpenSail)
    {
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Sail, 0, ElementsAre(0x11, 0x0))));

        EXPECT_CALL(this->_openSail, OpenSail());

        Run(0x11);
    }

    TEST_F(OpenSailTelecommandTest, ShouldRespondWithErrorFrameOnNoCorrelationId)
    {
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Sail, 0, ElementsAre(0x0, 0x1))));

        EXPECT_CALL(this->_openSail, OpenSail()).Times(0);

        Run();
    }
}
