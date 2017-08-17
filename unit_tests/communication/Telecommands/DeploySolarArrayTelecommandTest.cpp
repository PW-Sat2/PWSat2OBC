#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mission/sads.hpp"
#include "mock/DeploySolarArrayMock.hpp"
#include "mock/comm.hpp"
#include "obc/telecommands/sads.hpp"

using telecommunication::downlink::DownlinkAPID;
using testing::ElementsAre;

namespace
{
    class DeploySolarArrayTelecommandTest : public testing::Test
    {
      protected:
        template <typename... T> void Run(T... params);

        testing::NiceMock<TransmitterMock> _transmitter;
        testing::StrictMock<DeploySolarArrayMock> _deploySolarArray;

        obc::telecommands::DeploySolarArray _telecommand{_deploySolarArray};
    };

    template <typename... T> void DeploySolarArrayTelecommandTest::Run(T... params)
    {
        std::array<std::uint8_t, sizeof...(T)> buffer{static_cast<std::uint8_t>(params)...};

        this->_telecommand.Handle(_transmitter, buffer);
    }

    TEST_F(DeploySolarArrayTelecommandTest, ShouldDeploySolarArray)
    {
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x11, 0x0))));

        EXPECT_CALL(this->_deploySolarArray, DeploySolarArray());

        Run(0x11);
    }

    TEST_F(DeploySolarArrayTelecommandTest, ShouldRespondWithErrorFrameOnNoCorrelationId)
    {
        EXPECT_CALL(this->_transmitter, SendFrame(IsDownlinkFrame(DownlinkAPID::Operation, 0, ElementsAre(0x0, 0x1))));

        EXPECT_CALL(this->_deploySolarArray, DeploySolarArray()).Times(0);

        Run();
    }
}
