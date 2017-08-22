#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "mock/comm.hpp"
#include "obc/telecommands/periodic_message.hpp"

namespace
{
    struct TriggerPeriodicMessageMock : mission::ITriggerPeriodicMessage
    {
        MOCK_METHOD0(Trigger, void());
    };

    class SendPeriodicMessageTelecommandTest : public testing::Test
    {
      protected:
        testing::NiceMock<TransmitterMock> _transmitter;
        testing::NiceMock<TriggerPeriodicMessageMock> _trigger;

        obc::telecommands::SendPeriodicMessageTelecommand _telecommand{_trigger};
    };

    TEST_F(SendPeriodicMessageTelecommandTest, test)
    {
        EXPECT_CALL(_trigger, Trigger());

        _telecommand.Handle(_transmitter, {});
    }
}
